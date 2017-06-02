#include <atomic>
#include <cmath>

#include "neuron.h"

using namespace std;

atomic_int Neuron::unused_id(0);

/**
 * Creates neuron.
 *
 * @param output Will this be an output neuron? Takes precedent over inhibitory.
 * @param inhibitory Will this be an inhibitory neuron
 * @param initial_potential The initial potential of the neuron
 * @param fire_threshold Threshold at which this neuron will fire
 * @param disfacilitation Controls the extent to which neuron potential is
 *  lowered in down state transitions
 * @param ready_to_fire Function to be called when when the neuron is ready to
 *  fire
 */
Neuron::Neuron(
		bool output,
		bool inhibitory,
		double initial_potential,
		double fire_threshold,
		double disfacilitation,
		ready_callback ready_to_fire)
		: id(++unused_id)
		, threshold(fire_threshold)
		, disfacilitation(disfacilitation)
		, character(inhibitory ? -1 : 1)
		, is_out(output)
		, potential(initial_potential)
		, potential_prev(initial_potential)
		, depol(0)
		, weight_sum(0)
		, refractory(false)
		, refractory_prev(false)
		, active(false)
		, in_degree(0)
		, callback(ready_to_fire)
		, synapses() {
	; // Nothing else to see here
}

/**
 * If neuron is in a refractory state, potential will be reset and the
 * refractory state exited on the next turn. Otherwise, neuron will fire if it
 * is above potential, transmitting charge to all neurons to which it is
 * currently attached, and causing neuron to go into refractory state. Output
 * neurons won't actually fire. All neurons in the network should be reset
 * before this is called.
 *
 * @return Total of all depolarizations of all postsynaptic neurons this step
 * @see Neuron::reset()
 */
double Neuron::time_step(void) {
	double out_sum = 0;

	if (is_out) {
		potential = 0;
	} else if (refractory_prev) {
		refractory = false;
		potential = 0;
	} else if (potential_prev > threshold) {
		for (auto it = synapses.begin(); it != synapses.end(); it++)
			out_sum += it->second.fire(potential_prev, weight_sum);

		refractory = true;
		active = true;
	}

	return out_sum;
}

/**
 * Prepares the neuron for the next time step. This method should be called
 * on all neurons in the network before each call to time_step.
 *
 * @see Neuron::time_step()
 */
void Neuron::reset(void) {
	potential_prev = potential;
	refractory_prev = refractory;
}

/**
 * Implements Hebbian plasticity rules, increasing each synapse proportional to
 * its activity since the last call to this function.
 *
 * @param rate Learning rate.
 * @return Sum of all bond strength increases
 */
double Neuron::hebbian(double rate) {
	double increase_sum = 0;

	for (auto it = synapses.begin(); it != synapses.end(); it++)
		increase_sum += it->second.hebbian_increase(rate);

	renormalize_weights();

	return increase_sum;
}

/**
 * Strengthens the connection to the given neuron, creating the connection if
 * it does not yet exist, pruning connections as necessary.
 *
 * @param delta Amount by which to strengthen the connection
 * @param target Postsynaptic neuron
 */
void Neuron::strengthen_connection(double delta, Neuron &target) {
	try {
		if (synapses.at(target.get_id()).increase_strength(delta)) {
			synapses.erase(target.get_id());
		}
	} catch (out_of_range e) {
		synapses.insert(std::make_pair(target.get_id(),
			Synapse(*this, target, delta)));
	}

	renormalize_weights();
}

/**
 * Strengthens all of the existing outgoing connections of the neuron, pruning
 * as necessary.
 *
 * @param delta Amount by which to strengthen each connection
 * @return The sum of all weight strengths after updating
 */
double Neuron::strengthen_all_connections(double delta) {
	auto it = synapses.begin();
	while (it != synapses.end()) {
		if (it->second.increase_strength(delta)) {
			it = synapses.erase(it);
		} else {
			it++;
		}
	}

	renormalize_weights();

	return weight_sum;
}

/**
 * Gets the strength of the connection from this Neuron to the target Neuron.
 *
 * @param target The postsynaptic Neuron of the connection we're investigating
 * @return The strength of the connection to the target Neuron
 * @throws An out_of_range error if there is currently no connection to the
 *  target Neuron
 */
double Neuron::get_connection_strength(Neuron &target) const {
	return synapses.at(target.get_id()).get_strength();
}

/**
 * Transition to up state.
 *
 * @param potential Potential to which the neuron will be set
 */
void Neuron::go_up(double potential) {
	if (active)
		this->potential = potential;

	depol = 0;
	active = false;
}

/**
 * Transition to down state. Decreases neuron's potential proportional to sum of
 * depolarizations since last call to either this or go_up.
 *
 * @see Neuron::go_up(double);
 */
void Neuron::go_down() {
	if (active)
		potential -= disfacilitation * depol;

	depol = 0;
	active = false;
}

/**
 * Gets in degree of neuron.
 *
 * @return Number of incoming connections to this neuron
 */
int Neuron::get_in_degree(void) const {
	return in_degree;
}

/**
 * Gets out degree of neuron.
 *
 * @return Number of outgoing connections
 */
int Neuron::get_out_degree(void) const {
	return synapses.size();
}

/**
 * Gets the id of this neuron.
 *
 * @return An integer unique to this particular instantiation
 */
int Neuron::get_id(void) const {
	return id;
}

/**
 * Gets the firing threshold. Note that output neurons will not actually fire.
 *
 * @return The minimum potential at which the neuron will fire. Set at Neuron
 *  creation.
 * @see Neuron::Neuron(bool, bool, double, double, double, ready_callback)
 */
double Neuron::get_threshold(void) const {
	return threshold;
}

/**
 * Has the neuron fired since the last call to either go_up or go_down
 *
 * @return True if the neuron has fired
 * @see Neuron::go_up(double)
 * @see Neuron::go_down(double)
 */
bool Neuron::was_active(void) const {
	return active;
}

/**
 * Increases potential by given amount if not in a refractory state. If this
 * pushes potential over firing threshold, callback passed on creation will
 * be called. Output neurons will never be ready to fire, nor will neurons with
 * out degree 0.
 *
 * @param delta Amount by which to increase the potential. Can be negative.
 * @return Amount by which the potential increases
 * @see Neuron::Neuron(bool, bool, double, double, double, ready_callback)
 * @throws A runtime_exception if the increase causes the potential to become
 *  nonfinite
 */
double Neuron::increase_potential(double delta) {
	if (!refractory_prev) {
		potential += delta;
		depol += std::fabs(delta);

		if (potential > threshold && !is_out)
			callback(*this);

		if (!isfinite(potential))
			throw runtime_error("Nonfinite potential");

		return depol;
	}

	return 0;
}

/**
 * Get the excitatory or inhibitory character of the neuron.
 *
 * @return -1 if the neuron is inhibitory, 1 if exictatory
 */
int Neuron::get_character(void) {
	return character;
}

/**
 * Gets the new weight_sum after updates to outgoing synapse strengths.
 *
 * @see Neuron::strengthen_connection(double, Neuron&)
 * @see Neuron::strengthen_all_connections(double)
 * @see Neuron::hebbian()
 */
void Neuron::renormalize_weights(void) {
	weight_sum = 0;

	for (auto it = synapses.begin(); it != synapses.end(); it++)
		weight_sum += it->second.get_strength();
}