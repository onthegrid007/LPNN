#ifndef LPNN_NET_HPP_
#define LPNN_NET_HPP_

#include "neuron.hpp"
#include "vendor/Profiler/vendor/FPNBits/vendor/DynamicDLL/vendor/FileUtilities/vendor/STDExtras/vendor/ThreadPool/vendor/Semaphore/semaphore.h"
#include "vendor/Profiler/vendor/FPNBits/vendor/DynamicDLL/vendor/FileUtilities/vendor/STDExtras/vendor/ThreadPool/threadpool.h"

namespace LPNN {
	template<typename T>
	class Net : NonCopyable {
		public:
		typedef std::vector<std::pair<std::uint32_t, enum Layer<T>::ActivationFunction>> Topology;
		
		Net& out(std::vector<T>& out) const {
			out.clear();
			const Layer<T>& endLayer{m_layers.back()};
			for(const Neuron<T>& neuron : endLayer)
				out.emplace_back(neuron.m_output);
				return *this;
		}
		
		Net& backwardProp(const std::vector<T>& targets) {
			Layer<T>& endLayer{m_layers.back()};
			m_error = 0;
			const auto endSize{endLayer.size() - 1};
			std::lock_guard<std::mutex> lock(m_mtx);
			auto& tp{ThreadPool::GetInstanceByKey(m_name)};
			m_sem.set(endSize + 1);
			for(std::uint32_t n{0}; n < endSize; n++) {
				Neuron<T>& neuron{endLayer[n]};
				const T& target{targets[n]};
				const T delta{target - neuron.m_output};
				m_error += (delta * delta);
				tp.enqueue_work([&, d = delta]()->void{ neuron.calcGradients(d); m_sem -= 1; });
			}
			m_error = sqrt(m_error / endSize);
			m_averageError = (((m_averageError * m_averageSmoothingFactor) + m_error) / (m_averageSmoothingFactor + 1));
			m_sem.waitForI(0);
			const auto layerSize{m_layers.size()};
			const auto revSize{layerSize - 1};
			for(std::uint32_t layer{revSize}; layer > 1; layer--) {
				Layer<T>& currentLayer{m_layers[layer - 1]};
				const Layer<T>& nextLayer{m_layers[layer]};
				const T nleSum{errorSum(nextLayer)};
				m_sem.set(currentLayer.size());
				for(Neuron<T>& neuron : currentLayer)
					tp.enqueue_work([&]()->void{ neuron.calcGradients(nleSum); m_sem -= 1; });
				m_sem.waitForI(0);
			}
			for(std::uint32_t layer{layerSize}; layer > 1; layer--) {
				Layer<T>& prevLayer{m_layers[layer - 2]};
				Layer<T>& currentLayer{m_layers[layer - 1]};
				m_sem.set(currentLayer.size());
				for(Neuron<T>& neuron : currentLayer)
					tp.enqueue_work([&]()->void{ neuron.updateWeights(prevLayer); m_sem -= 1; });
				m_sem.waitForI(0);
			}
			return *this;
		}
		
		Net& forwardProp(const std::vector<T> in) {
			Layer<T>& beginLayer{m_layers.back()};
			assert(in.size() == beginLayer.size());
			const auto beginSize{beginLayer.size() - 1};
			for(std::uint32_t n{0}; n < beginSize; n++)
				beginLayer[n].m_output = in[n];
			const auto layers{m_layers.size()};
			std::lock_guard<std::mutex> lock(m_mtx);
			auto& tp{ThreadPool::GetInstanceByKey(m_name)};
			for(std::uint32_t layerNum{1}; layerNum < layers; layerNum++) {
				Layer<T>& currentLayer{m_layers[layerNum]};
				Layer<T>& prevLayer{m_layers[layerNum - 1]};
				m_sem.set(currentLayer.size());
				for(Neuron<T>& neuron : currentLayer)
					tp.enqueue_work([&]()->void{ neuron.forwardProp(prevLayer); m_sem -= 1; });
				m_sem.waitForI(0);
			}
			return *this;
		}
		
		Net& train(const std::vector<T>& i, const std::vector<T>& t) {
			return forwardProp(i).backwardProp(t);
		}
		
		Net& fire(const std::vector<T>& i, std::vector<T>& o) {
			return forwardProp(i).out(o);
		}
		
		Net(std::string name, const Topology& topology, const std::function<T()> weightFunction = [](){ return (T(rand()) / T(RAND_MAX)); }, int64_t workers = 8) :
			m_name(name),
			m_topology(topology),
			m_layers(topology.size()) {
			const auto layers{m_layers.size()};
			for(std::uint32_t layerNum{1}; layerNum < layers; layerNum++) {
				Layer<T>& currentLayer{m_layers[layerNum]};
				const auto& currentTop{m_topology[layerNum]};
				currentLayer.m_type = currentTop.second;
				const std::uint32_t numWeights{(layerNum == (layers - 1)) ? 0 : m_topology[layerNum + 1].first};
				const auto& numNeurons{currentTop.first};
				for(std::uint32_t n{0}; n < numNeurons; n++)
					currentLayer.emplace_back(numWeights, n, weightFunction);
				currentLayer.emplace_back(numWeights, numNeurons, [](){ return 1; }).m_output = 1;;
			}
			ThreadPool::CreateNewInstance(m_name, workers);
		}
		
		~Net() {
			ThreadPool::DeleteInstanceByKey(m_name);
		}
		
		private:
		std::string m_name;
		const Topology m_topology;
		T m_error;
		T m_averageError;
		T m_averageSmoothingFactor;
		std::vector<Layer<T>> m_layers;
		std::mutex m_mtx;
		Semaphore m_sem{0};
	};
}

#endif

/*
// #include <iostream>
// #include <cstdlib>
// #include <cassert>
// #include <cmath>
// #include <fstream>
// #include <sstream>



int main()
{
	TrainingData trainData("trainingData.txt");
	//e.g., {3, 2, 1 }
	vector<unsigned> topology;
	//topology.push_back(3);
	//topology.push_back(2);
	//topology.push_back(1);

	trainData.getTopology(topology);
	Net myNet(topology);

	vector<double> inputVals, targetVals, resultVals;
	int trainingPass = 0;
	while(!trainData.isEof())
	{
		++trainingPass;
		cout << endl << "Pass" << trainingPass;

		// Get new input data and feed it forward:
		if(trainData.getNextInputs(inputVals) != topology[0])
			break;
		showVectorVals(": Inputs :", inputVals);
		myNet.feedForward(inputVals);

		// Collect the net's actual results:
		myNet.getResults(resultVals);
		showVectorVals("Outputs:", resultVals);

		// Train the net what the outputs should have been:
		trainData.getTargetOutputs(targetVals);
		showVectorVals("Targets:", targetVals);
		assert(targetVals.size() == topology.back());

		myNet.backProp(targetVals);

		// Report how well the training is working, average over recnet
		cout << "Net recent average error: "
		     << myNet.getRecentAverageError() << endl;
	}

	cout << endl << "Done" << endl;

}
*/