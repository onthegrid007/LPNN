#ifndef LPNN_NEURON_HPP_
#define LPNN_NEURON_HPP_

#include "weight.hpp"
#include <functional>
#include <vector>
#include <cstdint>
#include <random>

namespace LPNN {
	template<typename T>
	class Neuron;
	
	template<typename T>
	class Layer : public std::vector<Neuron<T>> {
		public:
		enum ActivationFunction : std::uint8_t {
			SIGMOID = 0,
			TANH,
			CUSTOM
		};
		
		Layer(const enum ActivationFunction& type) : m_type(type) {}
		
		// private:
		enum ActivationFunction m_type;
	};
	
	template<typename T>
	class Neuron {
		public:
		Neuron(const std::uint32_t weights, const std::uint32_t index, const std::function<T()> WeightFunction = [](){ return (T(rand()) / T(RAND_MAX)); }) : m_output(WeightFunction()), m_index(index) {
			for(std::uint32_t w{0}; w < weights; w++)
				m_weights.emplace_back(WeightFunction());
		}
		
		Neuron& updateWeights(Layer<T>& prevLayer, const T learningRate = 0.15, const T momentum = 0.5) {
			const auto prevSize{prevLayer.size() - 1};
			for(std::uint32_t n{0}; n < prevSize; n++) {
				Neuron<T>& neuron{prevLayer[n]};
				auto& weight{neuron.m_weights[m_index]};
				const T oldDeltaWeight{weight.m_delta};
				weight.updateWeight((learningRate * neuron.m_output * m_gradient) + (momentum * oldDeltaWeight));
			}
			return *this;
		}
		
		const T errorSum(const Layer<T>& nextLayer) {
			T sum{0};
			const auto size{nextLayer.size() - 1};
			for(std::uint32_t n{0}; n < size; n++)
				sum += (m_weights[n].m_current * nextLayer[n].m_gradient);
			return sum;
		}
		
		Neuron& calcGradient(const T delta, const Layer<T>& layer) {
			m_gradient = (delta * ActivationFunctionDerivative(m_output, layer.m_type));
			return *this;
		}

		const T ActivationFunction(const T& value, const enum Layer<T>::ActivationFunction& type) {
			switch(type) {
				case Layer<T>::ActivationFunction::SIGMOID:
				def:
				// return sigmoid(value);
				return T(1.0) / (T(1.0) + exp((value >= 0) ? -value : value));
				
				case Layer<T>::ActivationFunction::TANH:
				return tanh(value);
				
				default:
				goto def;
			};
		}

		const T ActivationFunctionDerivative(const T& value, const enum Layer<T>::ActivationFunction& type) {
			switch(type) {
				case Layer<T>::ActivationFunction::SIGMOID:
				def:
				return value * (T(1.0) - value);
				
				case Layer<T>::ActivationFunction::TANH:
				return T(1.0) - (value * value);
				
				default:
				goto def;
			};
		}
		
		Neuron& forwardProp(const Layer<T>& prevLayer, const enum Layer<T>::ActivationFunction& type) {
			T sum{0};
			for(const Neuron<T>& neuron : prevLayer)
				sum += (neuron.m_output * neuron.m_weights[m_index].m_current);
			m_output = ActivationFunction(sum, type);
			return *this;
		}
		
		const std::vector<Weight<T>>& getWeights() { return m_weights; }
		
		// private:
		T m_output;
		std::uint32_t m_index;
		T m_gradient;
		std::vector<Weight<T>> m_weights;
	};
}

#endif