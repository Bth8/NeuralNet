#ifndef CONNECTOME_H
#define CONNECTOME_H

#include <vector>
#include <random>

#include "neuron.h"
#include "noise.h"
#include "power_law.h"

/**
 * Builds up a new neural network with various network parameters.
 */
class ConnectomeBuilder {
public:
	ConnectomeBuilder(
		int size,
		int max_firings,
		int refractory_period,
		double fire_threshold,
		double disfacilitation,
		double max_strength,
		double inhibitory_fraction,
		double output_fraction,
		double exponent,
		RNG &g);

	void operator()(
		std::vector<Neuron> &neurons,
		Neuron::ready_callback callback);

private:
	RNG &g;
	int size;
	int max_firings;
	int ref_time;
	double fire_threshold;
	double disfacilitation;
	double max_strength;

	std::bernoulli_distribution inhibit;
	std::bernoulli_distribution output;
	std::uniform_real_distribution<double> weight_dist;
	std::uniform_int_distribution<int> neuron_dist;
	power_law_distribution<int> out_dist;
};

#endif /*CONNECTOME_H*/
