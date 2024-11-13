module dataset

import network
import arrays

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
	println(d)

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

	mut neurons := []network.Neuron{}
	for _ in 0 .. neuron_count {
		neurons << network.Neuron{}
	}

	return network.Network{
		input_domain: input_devices
		output_range: output_devices
		neurons:      neurons
	}
}
