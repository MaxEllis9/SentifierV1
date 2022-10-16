/*
  ==============================================================================

    NeuralNetwork.cpp
    Created: 6 Jul 2022 12:32:08pm
    Author:  Max Ellis

  ==============================================================================
*/

#include "NeuralNetwork.h"

// constructor of neural network class
NeuralNetwork::NeuralNetwork(std::vector<uint> topology, Scalar learningRate)
{
  this->topology = topology;
  this->learningRate = learningRate;
  for (uint i = 0; i < topology.size(); i++) {
    // initialize neuron layers
    if (i == topology.size() - 1)
      neuronLayers.push_back(new RowVector(topology[i]));
    else
      neuronLayers.push_back(new RowVector(topology[i] + 1));

    // initialize cache and delta vectors
    cacheLayers.push_back(new RowVector(neuronLayers.size()));
    deltas.push_back(new RowVector(neuronLayers.size()));

    // vector.back() gives the handle to recently added element
    // coeffRef gives the reference of value at that place
    // (using this as we are using pointers here)
    if (i != topology.size() - 1) {
      neuronLayers.back()->coeffRef(topology[i]) = 1.0;
      cacheLayers.back()->coeffRef(topology[i]) = 1.0;
    }

    // initialize weights matrix
    if (i > 0) {
      if (i != topology.size() - 1) {
        weights.push_back(new Matrix(topology[i - 1] + 1, topology[i] + 1));
        weights.back()->setRandom();
        weights.back()->col(topology[i]).setZero();
        weights.back()->coeffRef(topology[i - 1], topology[i]) = 1.0;
      }
      else {
        weights.push_back(new Matrix(topology[i - 1] + 1, topology[i]));
        weights.back()->setRandom();
      }
    }
  }
};
