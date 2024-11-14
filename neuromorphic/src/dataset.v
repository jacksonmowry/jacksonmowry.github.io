module dataset

import network
import arrays

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
		population << n.make_w_hidden_layer(network.Hidden_Params{
			synapse_value_lower_bound:           -1
			num_synapse_upper_bound:             4
			hidden_neuron_threshold_upper_bound: 3
		})
	}
	for _ in 0 .. 10 {
		population << n.make_w_hidden_layer(network.Hidden_Params{
			synapse_value_lower_bound:  -1
			num_synapse_upper_bound:    4
			hidden_neurons_upper_bound: 4
		})
	}

	for mut network in population {
		network.initialize() or { panic(err) }
	}

	return population
}

pub fn (d Dataset) test_network(mut n network.Network) !int {
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
