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
	class Layer : private std::vector<Neuron<T>> {
		public:
		enum ActivationFunction : std::uint8_t {
			SIGMOID = 0,
			TANH,
			CUSTOM
		};
		
		private:
		template<T>
		friend class Net;
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
			for(Neuron& neuron : prevLayer) {
				auto& weight{neuron.m_outputWeights[m_index]};
				const T oldDeltaWeight{weight.deltaWeight};
				weight.updateWeight((learningRate * neuron.m_output * m_gradient) + (momentum * oldDeltaWeight));
			}
			return *this;
		}
		
		const T errorSum(const Layer<T>& nextLayer) const {
			T sum{0};
			const auto size{nextLayer.size() - 1};
			for(std::uint32_t n{0}; n < size; n++)
				sum += (m_weights[n].m_current * nextLayer[n].m_gradient);
			return sum;
		}
		
		Neuron& calcGradient(const T delta) {
			m_gradient = (delta * ActivationFunctionDerivative(m_output));
			return *this;
		}

		const T ActivationFunction(const T value, const enum Layer<T>::ActivationFunction& type, const std::function<T(const T&)> CustomFunc = [](const T& value){ return value; }) {
			switch(type) {
				case Layer<T>::ActivationFunction::SIGMOID:
				def:
				return sigmoid(value);
				
				case Layer<T>::ActivationFunction::TANH:
				return tanh(value);
				
				case Layer<T>::ActivationFunction::CUSTOM:
				return CustomFunc(value);
				
				default:
				goto def;
			};
		}

		const T ActivationFunctionDerivative(const T value, const enum Layer<T>::ActivationFunction& type, const std::function<T(const T&)> CustomFunc = [](const T& value){ return value; }) {
			switch(type) {
				case Layer<T>::ActivationFunction::SIGMOID:
				def:
				return value;
				
				case Layer<T>::ActivationFunction::TANH:
				return T(1.0) - (value * value);
				
				case Layer<T>::ActivationFunction::CUSTOM:
				return CustomFunc(value);
				
				default:
				goto def;
			};
		}
		
		Neuron& forwardProp(const Layer<T>& prevLayer, const enum Layer<T>::ActivationFunction& type, const std::function<T(const T&)> CustomFunc = [](const T& value){ return value; }) {
			T sum{0};
			for(Neuron& neuron : prevLayer)
				sum += (neuron.m_output * neuron.m_weights[m_index].m_current);
			m_output = ActivationFunction(sum, type, CustomFunc);
			return *this;
		}
		
		const std::vector<Weight<T>>& getWeights() { return m_weights; }
		
		private:
		template<T>
		friend class Net;
		T m_output;
		std::uint32_t m_index;
		T m_gradient;
		std::vector<Weight<T>> m_weights;
	};
}

#endif