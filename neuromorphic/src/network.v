module network

import datatypes
import arrays
import maps
import viz
import os
import rand

pub struct Synapse {
pub:
	value int
	delay u32
	from  string
}

pub struct Neuron {
pub:
	threshold int = 1
pub mut:
	pre_synapses []Synapse
	charges      []int @[skip] // Do not JSON encode/decode this field
}

fn (mut n Neuron) charge_to_threshold(timestep int) {
	n.charges[timestep] = n.threshold
}

fn (n Neuron) spiked(current_timestep int) bool {
	return n.charges[current_timestep] >= n.threshold
}

pub fn (mut n Neuron) add_synapse(s Synapse) {
	n.pre_synapses << s
}

pub enum InputType {
	buckets
	timescale
	pwm
	spike
}

pub struct Input_Unit {
pub:
	min_value  int
	max_value  int
	input_type InputType
	input_prop int
pub mut:
	neurons []string
}

pub enum OutputType {
	largest_count
	last_to_spike
	spike
}

pub struct Output_Unit {
pub:
	output_neurons u32
	output_type    OutputType
	neuron_names   ?[]string @[optional]
pub mut:
	neurons        []string
	firing_tracker []int @[skip]
}

pub struct Network {
pub:
	min_synapse_value   int = -10
	max_synapse_value   int = 10
	min_threshold_value int = -10
	max_threshold_value int = 10
	max_synapse_count   u32 = 100
	max_neuron_count    u32 = 100
	max_synapse_delay   u32 = 5
	end_to_end_time     u32 = 50
pub mut:
	input_domain     []Input_Unit
	output_range     []Output_Unit
	neurons          map[string]Neuron
	current_timestep u32 @[skip]
}

// pub fn (mut n Network) add_neuron(neuron Neuron) {
// 	n.neurons << neuron
// }

pub fn (mut n Network) spikes_snapshot() []string {
	return maps.filter(n.neurons, fn [n] (k string, v Neuron) bool {
		return v.charges[n.modded_timestep()] >= v.threshold
	}).keys()
}

fn (n Network) modded_timestep() u32 {
	return n.current_timestep % (n.max_synapse_delay + 1)
}

fn (mut n Network) bucket_input(input_value int, input_device Input_Unit) {
	bucket := (input_value - input_device.min_value) / ((
		input_device.max_value - input_device.min_value + 1) / input_device.input_prop)
	n.neurons[input_device.neurons[bucket]].charge_to_threshold(n.modded_timestep())
}

fn (mut n Network) timescale_input(input_value int, input_device Input_Unit) {
	normalize := input_value / ((input_device.max_value - input_device.min_value) / input_device.input_prop)
	for i in 0 .. normalize {
		n.neurons[input_device.neurons[0]].charge_to_threshold((n.modded_timestep() + i) % n.max_synapse_delay)
	}
}

fn (mut n Network) pwm_input(input_value int, input_device Input_Unit) {
	step := (input_device.max_value - input_device.min_value) / input_value
	for i := 0; i < input_device.input_prop; i += step {
		n.neurons[input_device.neurons[0]].charge_to_threshold((n.modded_timestep() + i) % n.max_synapse_delay)
	}
}

fn (mut n Network) spike_input(input_value int, input_device Input_Unit) {
	if input_value != 0 {
		n.neurons[input_device.neurons[0]].charge_to_threshold(n.modded_timestep())
	}
}

pub fn (mut n Network) input(input_values []int) ! {
	if input_values.len == 0 {
		return
	}

	if input_values.len != n.input_domain.len {
		return error('Network.input: given too many values, expected ${n.input_domain.len}, received ${input_values.len}')
	}
	for i, input_value in input_values {
		if n.input_domain[i].input_type != .spike {
			if input_value < n.input_domain[i].min_value {
				return error('Network.input: attempting to apply value that it too low for input_domain[${i}], ${input_value} < min (${n.input_domain[i].min_value})')
			}
			if input_value > n.input_domain[i].max_value {
				return error('Network.input: attempting to apply value that it too high for input_domain[${i}], ${input_value} > max (${n.input_domain[i].max_value})')
			}
		}

		match n.input_domain[i].input_type {
			.buckets { n.bucket_input(input_value, n.input_domain[i]) }
			.timescale { n.timescale_input(input_value, n.input_domain[i]) }
			.pwm { n.pwm_input(input_value, n.input_domain[i]) }
			.spike { n.spike_input(input_value, n.input_domain[i]) }
		}
	}
}

pub fn (mut n Network) output() ! {
	for mut output_device in n.output_range {
		for j, neuron in output_device.neurons {
			if n.neurons[neuron].charges[n.modded_timestep()] >= n.neurons[neuron].threshold {
				match output_device.output_type {
					.largest_count {
						output_device.firing_tracker[j]++
					}
					.last_to_spike {
						output_device.firing_tracker[0] = j
					}
					.spike {
						output_device.firing_tracker[0] = 1
					}
				}
			} else {
				if output_device.output_type == .spike {
					output_device.firing_tracker[0] = 0
				}
			}
		}
	}
}

pub fn (n Network) format_output() ![]string {
	return n.output_range.map(fn (output_device Output_Unit) string {
		match output_device.output_type {
			.largest_count {
				if output_device.neuron_names != none {
					return output_device.neuron_names?[arrays.idx_max(output_device.firing_tracker) or {
						0
					}]
				} else {
					return output_device.neurons[arrays.idx_max(output_device.firing_tracker) or {
						0
					}].str()
				}
			}
			.last_to_spike {
				if output_device.neuron_names != none {
					return output_device.neuron_names?[output_device.firing_tracker[0]]
				} else {
					return output_device.neurons[output_device.firing_tracker[0]].str()
				}
			}
			.spike {
				if output_device.firing_tracker[0] != 0 {
					if output_device.neuron_names != none {
						return output_device.neuron_names?[0]
					} else {
						return '1'
					}
				}
			}
		}
	})
}

pub fn (mut n Network) reset_output() {
	for mut output_device in n.output_range {
		match output_device.output_type {
			.largest_count {
				for i in 0 .. output_device.neurons.len {
					output_device.firing_tracker[i] = 0
				}
			}
			.last_to_spike {
				output_device.firing_tracker[0] = -1
			}
			.spike {
				output_device.firing_tracker[0] = 0
			}
		}
	}
}

pub fn (mut n Network) initialize() ! {
	// Init neurons
	for _, mut neuron in n.neurons {
		for _ in 0 .. n.max_synapse_delay + 1 {
			neuron.charges << 0
		}
	}

	// Init output devices
	for mut output_device in n.output_range {
		match output_device.output_type {
			.largest_count {
				for _ in 0 .. output_device.neurons.len {
					output_device.firing_tracker << 0
				}
			}
			.last_to_spike {
				output_device.firing_tracker << -1
			}
			.spike {
				output_device.firing_tracker << 0
			}
		}
	}
}

pub fn (mut n Network) run() ! {
	for _, mut neuron in n.neurons {
		for synapse in neuron.pre_synapses {
			if n.neurons[synapse.from].charges[n.modded_timestep()] >= n.neurons[synapse.from].threshold {
				neuron.charges[(n.modded_timestep() + synapse.delay) % (n.max_synapse_delay + 1)] += synapse.value
			}
		}
	}

	for _, mut neuron in n.neurons {
		neuron.charges[n.modded_timestep()] = 0
	}
}

pub fn (n Network) verify_graph() ! {
	// Check min/max are in the correct order
	if n.min_synapse_value > n.max_synapse_value {
		return error('min_synapse_value is greater and max_synapse_value, ${n.min_synapse_value} > ${n.max_synapse_value}')
	}
	if n.min_threshold_value > n.max_threshold_value {
		return error('min_threshold_value is greater and max_threshold_value, ${n.min_threshold_value} > ${n.max_threshold_value}')
	}

	if n.max_synapse_count == 0 {
		return error('max_synapse_count is 0')
	}

	if n.max_neuron_count == 0 {
		return error('max_neuron_count is 0')
	}

	if n.end_to_end_time < n.max_synapse_delay {
		return error('end_to_end_time is less than the networks max_synapse_delay, ${n.end_to_end_time} < ${n.max_synapse_delay}')
	}

	// Sanity check each neuron and it's pre-synapses
	for i, neuron in n.neurons {
		if neuron.threshold > n.max_threshold_value || neuron.threshold < n.min_threshold_value {
			return error('Neuron ${i} threshold out of range should be between ${n.min_threshold_value} and ${n.max_threshold_value}, is ${neuron.threshold}')
		}

		for j, synapse in neuron.pre_synapses {
			if synapse.value > n.max_synapse_value || synapse.value < n.min_synapse_value {
				return error('Synapse ${j} on neuron ${i} value out of range should be between ${n.min_synapse_value} and ${n.max_synapse_value}, is ${synapse.value}')
			}
			if synapse.delay > n.max_synapse_delay {
				return error('Synapse ${j} on neuron ${i} delay out of should be less or equal to ${n.max_synapse_delay}, is ${synapse.delay}')
			}
			if synapse.from !in n.neurons {
				return error('Synapse ${j} on neuron ${i} out of range ${n.neurons.len} exist in the network, yet synapse is referencing ${synapse.from}')
			}
		}
	}

	if n.input_domain.len == 0 {
		return error('Networks set of inputs is empty')
	}

	if n.output_range.len == 0 {
		return error('Networks set of outputs is empty')
	}

	// Check if individual input devices overlap
	all_input_neurons := arrays.flatten(n.input_domain.map(it.neurons))
	if all_input_neurons.sorted() != arrays.distinct(all_input_neurons) {
		return error('Input domain devices contain overlapping neurons: ${n.input_domain.map(it.neurons)}')
	}
	// Check if individual output devices overlap
	all_output_neurons := arrays.flatten(n.output_range.map(it.neurons))
	if all_output_neurons != arrays.distinct(all_output_neurons) {
		return error('Output range devices contain overlapping neurons: ${n.output_range.map(it.neurons)}')
	}

	// Check if inputs/output overlap
	mut input_set := datatypes.Set[string]{}
	input_set.add_all(arrays.flatten(n.input_domain.map(it.neurons)))
	mut output_set := datatypes.Set[string]{}
	output_set.add_all(arrays.flatten(n.output_range.map(it.neurons)))

	intersection_set := input_set.intersection(output_set)

	if intersection_set.size() != 0 {
		return error('Inputs and Outputs overlap, ${intersection_set.elements.keys()}')
	}

	mut needed_input_neurons := 0
	for i, input_device in n.input_domain {
		if input_device.min_value > input_device.max_value {
			return error('Element ${i} in input_domain has a min_value greater than its max_value, ${input_device.min_value} > ${input_device.max_value}')
		}
		if input_device.input_type == .buckets {
			if input_device.input_prop == 0 {
				return error('Element ${i} in input_domain has a bucket count of 0')
			}
			if input_device.input_prop != (input_device.max_value - input_device.min_value + 1) {
				return error('Element ${i} in input_domain has more buckets than its specified range allows, ${input_device.input_prop} buckets with a range of ${
					input_device.max_value - input_device.min_value + 1}')
			}
			needed_input_neurons += input_device.input_prop
		} else {
			if input_device.input_prop == 0 {
				return error('Element ${i} in input_domain has a timescale value of 0')
			}
			if input_device.input_prop > n.max_synapse_delay {
				return error('Element ${i} in input_domain has a timescale value greater than the max delay, ${input_device.input_prop} > ${n.max_synapse_delay}')
			}
			needed_input_neurons++
		}
	}

	if needed_input_neurons != input_set.size() {
		return error('Mismatched size of inputs and input_domain, inputs=${input_set.size()} and input_domain=${needed_input_neurons}')
	}

	mut needed_output_neurons := 0
	for i, output_device in n.output_range {
		if output_device.output_neurons == 0 {
			return error('Element ${i} in output_range has a output_neurons count of 0')
		}
		needed_output_neurons += output_device.output_neurons
	}

	if needed_output_neurons != output_set.size() {
		return error('Mismatched size of outputs and output_range, outputs=${output_set.size()} and output_range=${needed_output_neurons}')
	}
}

pub fn (n Network) visualize() ! {
	mut d := viz.new('n_viz', 'Network', 'blue')
	// for i, neuron in n.neurons {
	// 	d.new_node('${i} (${neuron.threshold})', '${i}')
	// }

	// Input cluster
	d.writeln('subgraph cluster_input {')
	d.writeln('label = "Input Domain";')
	for i, input_group in n.input_domain {
		d.writeln('subgraph cluster_${i}_in {')
		d.writeln('label = "Input Block ${i}";')
		for neuron in input_group.neurons {
			d.new_node('${neuron} (${n.neurons[neuron].threshold})', neuron)
		}
		d.writeln('}')
	}
	d.writeln('}')
	// Hidden cluster
	d.writeln('subgraph cluster_hidden {')
	d.writeln('label = "Hidden Neurons";')
	hidden_neurons := n.neurons.keys().filter(it !in arrays.flatten(n.input_domain.map(it.neurons))
		&& it !in arrays.flatten(n.output_range.map(it.neurons)))

	for neuron in hidden_neurons {
		d.new_node('${neuron} (${n.neurons[neuron].threshold})', neuron)
	}

	d.writeln('}')
	// Output cluster
	d.writeln('subgraph cluster_output {')
	d.writeln('label = "Output Range";')
	for i, output_group in n.output_range {
		d.writeln('subgraph cluster_${i}_out {')
		d.writeln('label = "Output Block ${i}";')
		for neuron in output_group.neurons {
			d.new_node('${neuron} (${n.neurons[neuron].threshold})', neuron)
		}
		d.writeln('}')
	}
	d.writeln('}')

	for i, neuron in n.neurons {
		for s in neuron.pre_synapses {
			d.new_edge('${s.from}', '${i}', if s.value > 0 { 'black' } else { 'red' })
		}
	}

	sixel_graph := os.system("echo 'digraph G {\n${d.sb.str()}\n}'| dot -Tpng | img2sixel")
	if sixel_graph != 0 {
		return error('Viz not supported on this platform, please try installing `graphviz (dot)` and `img2sixel`')
	}
	println('')
}

pub fn (n Network) make_fully_connected() Network {
	mut new_network := n.clone()
	input_neurons := arrays.flatten(new_network.input_domain.map(it.neurons))
	output_neurons := arrays.flatten(new_network.output_range.map(it.neurons))

	for output_neuron in output_neurons {
		for input_neuron in input_neurons {
			new_network.neurons[output_neuron].pre_synapses << Synapse{
				value: 1
				delay: 1
				from:  input_neuron
			}
		}
	}

	return new_network
}

@[params]
pub struct Sparse_Params {
pub:
	synapse_value_lower_bound i32 = 1
	synapse_value_upper_bound i32 = 1
	num_synapse_upper_bound   u32 = 1
	delay_upper_bound         u32 = 1
}

pub fn (n Network) make_sparsly_connected(params Sparse_Params) Network {
	mut new_network := n.clone()
	input_neurons := arrays.flatten(new_network.input_domain.map(it.neurons))
	output_neurons := arrays.flatten(new_network.output_range.map(it.neurons))
	mut values_wo_zero := []int{}
	for i in params.synapse_value_lower_bound .. params.synapse_value_upper_bound + 1 {
		if i != 0 {
			values_wo_zero << i
		}
	}

	for output_neuron in output_neurons {
		for _ in 0 .. rand.u32_in_range(1, params.num_synapse_upper_bound) or { 1 } {
			new_network.neurons[output_neuron].pre_synapses << Synapse{
				value: rand.element(values_wo_zero) or { 1 }
				delay: rand.u32_in_range(1, params.delay_upper_bound + 1) or { 1 }
				from:  rand.element(input_neurons) or { input_neurons[0] }
			}
		}
	}

	return new_network
}

@[params]
pub struct Hidden_Params {
pub:
	synapse_value_lower_bound           i32 = 1
	synapse_value_upper_bound           i32 = 1
	num_synapse_upper_bound             u32 = 1
	delay_upper_bound                   u32 = 1
	hidden_neurons_lower_bound          u32 = 1
	hidden_neurons_upper_bound          u32 = 1
	hidden_neuron_threshold_lower_bound u32 = 1
	hidden_neuron_threshold_upper_bound u32 = 1
}

pub fn (n Network) make_w_hidden_layer(params Hidden_Params) Network {
	mut new_network := n.clone()
	mut values_wo_zero := []int{}
	for i in params.synapse_value_lower_bound .. params.synapse_value_upper_bound + 1 {
		if i != 0 {
			values_wo_zero << i
		}
	}
	input_neurons := arrays.flatten(new_network.input_domain.map(it.neurons))
	output_neurons := arrays.flatten(new_network.output_range.map(it.neurons))
	mut hidden_neurons := []string{}
	mut highest_neuron := arrays.max(new_network.neurons.keys().map(it.int())) or { 10 } + 1

	for _ in 0 .. rand.u32_in_range(params.hidden_neurons_lower_bound,
		params.hidden_neurons_upper_bound + 1) or { 1 } {
		hidden_neurons << '${highest_neuron}'
		new_network.neurons['${highest_neuron++}'] = Neuron{
			threshold: rand.u32_in_range(params.hidden_neuron_threshold_lower_bound,
				params.hidden_neuron_threshold_upper_bound + 1) or { 1 }
		}
	}

	for neuron in hidden_neurons {
		for from in input_neurons {
			new_network.neurons[neuron].pre_synapses << Synapse{
				value: rand.element(values_wo_zero) or { 1 }
				delay: rand.u32_in_range(1, params.delay_upper_bound + 1) or { 1 }
				from:  from
			}
		}
	}

	for neuron in output_neurons {
		for from in hidden_neurons {
			new_network.neurons[neuron].pre_synapses << Synapse{
				value: rand.element(values_wo_zero) or { 1 }
				delay: rand.u32_in_range(1, params.delay_upper_bound + 1) or { 1 }
				from:  from
			}
		}
	}

	return new_network
}

pub fn (n Network) clone() Network {
	return Network{
		input_domain: n.input_domain.clone()
		output_range: n.output_range.clone()
		neurons:      n.neurons.clone()
	}
}
