module dataset

import network
import arrays
import rand

pub struct Test_Result {
pub:
	score   int
	network network.Network
}

struct Labeled_Data {
pub:
	input  []int
	output []string
}

struct Dataset {
	input_dimension  int
	output_dimension int
	expected_outputs [][]string
pub:
	data []Labeled_Data
}

pub fn (d Dataset) validate_dataset() ! {
	if !d.data.all(it.input.len == d.input_dimension) {
		return error('Not all input data matches input_dimension')
	}

	if !d.data.all(it.output.len == d.output_dimension) {
		return error('Not all output data matches output_dimension')
	}

	if d.expected_outputs.len != d.output_dimension {
		return error('Length of expected output does not match output_dimension')
	}
}

pub fn (d Dataset) create_skeleton_network() !network.Network {
	mut neuron_count := 0
	mut input_devices := []network.Input_Unit{}
	for i in 0 .. d.input_dimension {
		min := arrays.min(d.data.map(it.input[i]))!
		max := arrays.max(d.data.map(it.input[i]))!
		input_devices << network.Input_Unit{
			min_value:  min
			max_value:  max
			input_type: .buckets
			input_prop: (max - min) + 1
		}
		for _ in 0 .. (max - min) + 1 {
			input_devices.last().neurons << '${neuron_count++}'
		}
	}

	mut output_devices := []network.Output_Unit{}
	for i in 0 .. d.output_dimension {
		output_devices << network.Output_Unit{
			output_neurons: u32(d.expected_outputs[i].len)
			output_type:    .largest_count
			neuron_names:   d.expected_outputs[i]
		}

		for _ in 0 .. d.expected_outputs[i].len {
			output_devices.last().neurons << '${neuron_count++}'
		}
	}

	mut neurons := map[string]network.Neuron{}
	for i in 0 .. neuron_count {
		neurons['${i}'] = network.Neuron{}
	}

	return network.Network{
		input_domain: input_devices
		output_range: output_devices
		neurons:      neurons
	}
}

pub fn (d Dataset) generate_initial_population(n network.Network) []network.Network {
	// Create a fully connected input and output layer with simple delay 1 value 1 synapses
	mut population := []network.Network{}
	population << n.make_fully_connected()
	for _ in 0 .. 10 {
		population << n.make_sparsly_connected()
	}
	for _ in 0 .. 10 {
		population << n.make_sparsly_connected(network.Sparse_Params{ synapse_value_lower_bound: -1 })
	}
	for _ in 0 .. 10 {
		population << n.make_sparsly_connected(network.Sparse_Params{ num_synapse_upper_bound: 3 })
	}
	for _ in 0 .. 10 {
		population << n.make_sparsly_connected(network.Sparse_Params{ num_synapse_upper_bound: 4 })
	}
	for _ in 0 .. 10 {
		population << n.make_sparsly_connected(network.Sparse_Params{ delay_upper_bound: 3 })
	}
	for _ in 0 .. 10 {
		population << n.make_sparsly_connected(network.Sparse_Params{
			synapse_value_lower_bound: -1
			synapse_value_upper_bound: 2
			num_synapse_upper_bound:   4
		})
	}
	for _ in 0 .. 10 {
		population << n.make_sparsly_connected(network.Sparse_Params{
			synapse_value_lower_bound: -1
			synapse_value_upper_bound: 2
			num_synapse_upper_bound:   2
			recurrence_denominator:    5
		})
	}
	for _ in 0 .. 10 {
		population << n.make_w_hidden_layer(network.Hidden_Params{
			synapse_value_lower_bound:           -1
			num_synapse_upper_bound:             4
			hidden_neuron_threshold_upper_bound: 3
		})
	}
	for _ in 0 .. 10 {
		population << n.make_w_hidden_layer(network.Hidden_Params{
			synapse_value_lower_bound:           -1
			num_synapse_upper_bound:             2
			hidden_neuron_threshold_upper_bound: 3
			recurrence_denominator:              5
		})
	}
	for _ in 0 .. 10 {
		population << n.make_w_hidden_layer(network.Hidden_Params{
			synapse_value_lower_bound:  -1
			num_synapse_upper_bound:    4
			hidden_neurons_upper_bound: 4
		})
	}

	for i, mut network in population {
		network.name = 'Initial ${i}'
		network.initialize() or { panic(err) }
	}

	return population
}

pub fn (d Dataset) generate_next_population(networks []network.Network) []network.Network {
	mut new_population := []network.Network{}
	for i, n in networks {
		mut new_network := n.clone()
		for _, mut neuron in new_network.neurons {
			for mut synapse in neuron.pre_synapses {
				// Value Flip
				if rand.u32n(25) or { 25 } == 0 {
					synapse.value *= -1
				}
				// Value Bump
				if rand.u32n(25) or { 25 } == 0 {
					synapse.value += rand.element([-1, 1]) or { 1 }
					if synapse.value == 0 {
						synapse.value = 1
					}
				}
				// Delay Bump
				if rand.u32n(25) or { 25 } == 0 {
					synapse.delay += rand.element([u32(1), 1]) or { 1 }
				}
				// Synapse Deletion
				if rand.u32n(25) or { 25 } == 0 {
					synapse.value = 0
				}
			}

			// Synapse Addition
			if rand.u32n(10) or { 10 } == 0 {
				from := rand.element(new_network.neurons.keys()) or { '0' }
				to := rand.element(new_network.neurons.keys()) or { '0' }

				new_network.neurons[to].pre_synapses << network.Synapse{
					value: 1
					delay: 1
					from:  from
				}
			}

			neuron.pre_synapses = neuron.pre_synapses.filter(it.value != 0)
		}

		new_network.name = 'Next ${i}'
		new_population << new_network
	}

	return new_population.clone()
}

pub fn (d Dataset) generate_random_batch() []network.Network {
	n := d.create_skeleton_network() or { panic(err) }
	// Create a fully connected input and output layer with simple delay 1 value 1 synapses
	mut population := []network.Network{}
	population << n.make_fully_connected()
	for _ in 0 .. 10 {
		population << n.make_sparsly_connected(network.Sparse_Params{
			synapse_value_lower_bound: -1
			num_synapse_upper_bound:   3
			delay_upper_bound:         3
			recurrence_denominator:    5
		})
	}
	for _ in 0 .. 10 {
		population << n.make_w_hidden_layer(network.Hidden_Params{
			synapse_value_lower_bound:           -1
			num_synapse_upper_bound:             4
			hidden_neuron_threshold_upper_bound: 3
			recurrence_denominator:              5
		})
	}

	for i, mut network in population {
		network.name = 'Random Batch ${i}'
		network.initialize() or { panic(err) }
	}

	return population
}

pub fn (d Dataset) test_network(mut n network.Network) !int {
	n.initialize()!
	mut correct_runs := 0
	main_loop: for data in d.data {
		if data.input.len != n.input_domain.len && data.input.len != 0 {
			eprintln('Mismatched number of inputs, expected ${n.input_domain.len} got ${data.input.len}')
			continue
		}

		n.input(data.input) or {
			eprintln(err.msg())
			continue
		}

		for _ in 0 .. n.end_to_end_time {
			n.run() or {
				eprintln(err.msg())
				exit(1)
			}
			n.current_timestep++
			n.output()!
		}

		if n.format_output()! == data.output {
			correct_runs++
		}
		n.reset_output()
	}

	return correct_runs
}
