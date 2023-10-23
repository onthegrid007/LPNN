#ifndef LPNN_NET_HPP_
#define LPNN_NET_HPP_

#include "neuron.hpp"
// #include "assert.h"
#include <iostream>
#include "vendor/Profiler/vendor/FPNBits/vendor/DynamicDLL/vendor/FileUtilities/vendor/STDExtras/vendor/ThreadPool/threadpool.hpp"

#define BADLOV(var) std::cout << #var << ": " << var << std::endl
#define BADLOT(text) std::cout << text << << std::endl

namespace LPNN {
	template<typename T>
	class Net : NonCopyable {
		public:
				T m_error;
				T m_averageError;

		typedef std::vector<std::pair<std::uint32_t, enum Layer<T>::ActivationFunction>> Topology;
		
		void out(std::vector<T>& out) const {
			out.clear();
			// fix to no include bias
			const Layer<T>& endLayer{m_layers.back()};
			for(const Neuron<T>& neuron : endLayer)
				out.emplace_back(neuron.m_output);
		}
		
		Net& backwardProp(const std::vector<T>& targets) {
			std::lock_guard<std::mutex> lock(m_mtx);
			Layer<T>& endLayer{m_layers.back()};
			m_error = 0;
			// BADLOV(m_error);
			const auto endSize{endLayer.size() - 1};
			m_sem.reset(endSize);
			for(std::uint32_t n{0}; n < endSize; n++) {
				Neuron<T>& neuron{endLayer[n]};
				const T delta{targets[n] - neuron.m_output};
				m_error += (delta * delta);
				// BADLOV(m_error);
				// BADLOV(delta);
				// neuron.calcGradient(delta, endLayer);
				m_tp.enqueue_work<false>([&, d = delta](){ neuron.calcGradient(d, endLayer); m_sem.dec(); }, ThreadPool::EnqueuePriority::DEFAULT);
			}
			m_averageError = (((m_averageError * m_averageSmoothingFactor) + (m_error = sqrt(m_error /= endSize))) / (m_averageSmoothingFactor + 1));
			m_sem.waitForC(0);
			const auto netSize{m_layers.size() - 1};
			const auto hiddenSize{netSize - 1};
			for(auto layer{hiddenSize}; layer > 0; layer--) {
				Layer<T>& currentLayer{m_layers[layer]};
				const Layer<T>& nextLayer{m_layers[layer + 1]};
				// const auto currentSize{currentLayer.size() - 1};
				m_sem.reset(currentLayer.size());
				// for(std::uint32_t n{0}; n < currentSize; n++) {
					// Neuron<T>& neuron{currentLayer[n]};
				for(Neuron<T>& neuron : currentLayer) {
					// neuron.calcGradient(neuron.errorSum(nextLayer), currentLayer);
					m_tp.enqueue_work<false>([&](){ neuron.calcGradient(neuron.errorSum(nextLayer), currentLayer); m_sem.dec(); }, ThreadPool::EnqueuePriority::DEFAULT);
				}
				m_sem.waitForC(0);
			}
			for(auto layer{netSize}; layer > 0; layer--) {
				Layer<T>& currentLayer{m_layers[layer]};
				Layer<T>& prevLayer{m_layers[layer - 1]};
				const auto currentSize{currentLayer.size() - 1};
				m_sem.reset(currentSize);
				for(std::uint32_t n{0}; n < currentSize; n++) {
					// currentLayer[n].updateWeights(prevLayer);
					m_tp.enqueue_work<false>([&, idx = n](){ currentLayer[idx].updateWeights(prevLayer); m_sem.dec(); }, ThreadPool::EnqueuePriority::DEFAULT);
				}
				m_sem.waitForC(0);
				// while(int64_t(m_sem) > 0) std::this_thread::yield();
			}
			return *this;
		}
		
		Net& forwardProp(const std::vector<T> in) {
			std::lock_guard<std::mutex> lock(m_mtx);
			Layer<T>& beginLayer{m_layers[0]};
			// assert(in.size() == beginLayer.size());
			const auto beginSize{beginLayer.size() - 1};
			for(std::uint32_t n{0}; n < beginSize; n++)
				beginLayer[n].m_output = in[n];
			const auto layers{m_layers.size()};
			for(std::uint32_t layerNum{1}; layerNum < layers; layerNum++) {
				Layer<T>& currentLayer{m_layers[layerNum]};
				Layer<T>& prevLayer{m_layers[layerNum - 1]};
				const auto curentSize{currentLayer.size() - 1};
				m_sem.reset(curentSize);
				for(std::uint32_t n{0}; n < curentSize; n++) {
					// currentLayer[n].forwardProp(prevLayer, currentLayer.m_type);
					m_tp.enqueue_work<false>([&, idx = n](){ currentLayer[idx].forwardProp(prevLayer, currentLayer.m_type); m_sem.dec(); }, ThreadPool::EnqueuePriority::DEFAULT);
				}
				m_sem.waitForC(0);
				// while(int64_t(m_sem) > 0) std::this_thread::yield();
			}
			return *this;
		}
		
		Net& train(const std::vector<T>& i, const std::vector<T>& t) {
			return forwardProp(i).backwardProp(t);
		}
		
		Net& fire(const std::vector<T>& i, std::vector<T>& o) {
			return forwardProp(i).out(o);
		}
		
		Net(const Topology& topology, const std::function<T()> weightFunction = [](){ return (T(rand()) / T(RAND_MAX)); }, int64_t workers = 8) :
			m_topology(topology),
			m_layers(),
			m_tp(workers, ThreadPool::QueueProcedure::NONBLOCKING, 0) {
			srand(time(0));
			const auto layers{topology.size()};
			for(std::uint32_t layerNum{0}; layerNum < layers; layerNum++) {
				const auto& currentTop{m_topology[layerNum]};
				Layer<T>& currentLayer{m_layers.emplace_back(currentTop.second)};
				const std::uint32_t numWeights{(layerNum == (layers - 1)) ? 0 : m_topology[layerNum + 1].first};
				const auto& numNeurons{currentTop.first};
				for(std::uint32_t n{0}; n < numNeurons; n++)
					currentLayer.emplace_back(numWeights, n, weightFunction);
				currentLayer.emplace_back(numWeights, numNeurons, [](){ return T(1); }).m_output = 1;
			}
			// ThreadPool::CreateNewInstance(m_name, workers);
		}
		
		~Net() {
			// ThreadPool::DeleteInstanceByKey(m_name);
		}
		
		private:
		std::string m_name;
		const Topology m_topology;
		// T m_error;
		// T m_averageError;
		T m_averageSmoothingFactor = 100.0;
		std::vector<Layer<T>> m_layers;
		std::mutex m_mtx;
		Semaphore m_sem{0};
		ThreadPool m_tp;
	};
}

#endif