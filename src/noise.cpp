#include <random>

#include "noise.h"

/**
 * Creates a NeuronNoise object
 *
 * @param g The RNG from whence the noise will be derived
 * @param mean The mean of the normal distribution used for down states
 * @param stdev The standard deviation of the distribution used for down states
 * @see NeuronNoise::operator()(std::vector<neurons>&)
 */
NeuronNoise::NeuronNoise(double mean, double stdev, RNG &g)
		: g(g)
		, down_dist(mean, stdev) {
	; // Nothing to see here
}

/**
 * Apply neuron noise for up states. Stimulates one random neuron with potential
 * increase uniformly distributed on the interval [0, range).
 *
 * @param neurons Vector containing neurons to be stimulated
 * @param range Determines the range of possible stimulations
 */
void NeuronNoise::operator()(std::vector<Neuron> &neurons, double range) {
	std::uniform_int_distribution<int> neuron_dist(0, neurons.size() - 1);
	std::uniform_real_distribution<double> noise_dist(0, range);

	neurons.at(neuron_dist(g)).increase_potential(noise_dist(g));
}

/**
 * Apply neuron noise for down states. Stimulates one random neuron with
 * potential increase normally distributed according to parameters passed at
 * object creation.
 *
 * @param neurons Vector containing neurons to be stimulated
 * @see NeuronNoise::NeuronNoise(double, double, RNG)
 */
void NeuronNoise::operator()(std::vector<Neuron> &neurons) {
	std::uniform_int_distribution<int> neuron_dist(0, neurons.size() - 1);

	neurons.at(neuron_dist(g)).increase_potential(down_dist(g));
}

/**
 * Creates a WeightNoise object.
 *
 * @param g The source of randomness for the noise
 * @param mean The average amount of noise added to each connection
 * @param stdev The standard deviation of the noise added to each connection
 */
 WeightNoise::WeightNoise(double mean, double stdev, RNG &g)
 		: g(g)
 		, noise_dist(mean, stdev) {
 	; // Nothing to see here
 }

/**
 * Applies weight noise to the given list of neurons. Randomly strengthens or
 * weakens connections between neurons in the vector. May create new connections
 * or destroy old ones.
 *
 * @param neurons Vector containing neurons whose mutual connections will be
 *  strengthened or weakened
 */
/*WeightNoise::operator()(vector<Neurons> &neurons) {
	TODO
}*/
