module main

import json
import os
import cli
import viz
import network
import strconv
import dataset

fn main() {
	mut app := cli.Command{
		name:        'neuromorphic'
		description: 'neuromorphic runtime'
	}

	mut visualize_cmd := cli.Command{
		name:          'visualize'
		description:   'Pretty Print the Network'
		usage:         '<network json>'
		required_args: 0
		execute:       visualize_func
	}

	visualize_cmd.add_flag(cli.Flag{
		flag:        .string
		required:    true
		name:        'json'
		abbrev:      'n'
		description: 'Network json file'
	})

	app.add_command(visualize_cmd)

	mut run_cmd := cli.Command{
		name:          'run'
		description:   'Run the network using stdin/stdout'
		usage:         '<network json>'
		required_args: 0
		execute:       run_func
	}

	run_cmd.add_flag(cli.Flag{
		flag:        .string
		required:    true
		name:        'json'
		abbrev:      'n'
		description: 'Network json file'
	})

	app.add_command(run_cmd)

	mut train_classification_cmd := cli.Command{
		name:          'train_classification'
		description:   'Train a network based upon a given dataset'
		usage:         '<dataset json>'
		required_args: 0
		execute:       train_classification_func
	}

	train_classification_cmd.add_flag(cli.Flag{
		flag:        .string
		required:    true
		name:        'dataset'
		abbrev:      'd'
		description: 'Dataset json file'
	})

	app.add_command(train_classification_cmd)

	mut test_classification_cmd := cli.Command{
		name:          'test_classification'
		description:   'Test a network based upon a given dataset'
		usage:         '<dataset json>'
		required_args: 0
		execute:       test_classification_func
	}

	test_classification_cmd.add_flag(cli.Flag{
		flag:        .string
		required:    true
		name:        'network'
		abbrev:      'n'
		description: 'Network json file'
	})

	test_classification_cmd.add_flag(cli.Flag{
		flag:        .string
		required:    true
		name:        'dataset'
		abbrev:      'd'
		description: 'Dataset json file'
	})

	app.add_command(test_classification_cmd)

	app.setup()
	app.parse(os.args)
}

fn visualize_func(cmd cli.Command) ! {
	filename := cmd.flags.get_string('json') or { panic('Failed to get network json file: ${err}') }

	network_json := os.read_file(filename) or { panic('Failed to read_file ${filename}, ${err}') }

	n := json.decode(network.Network, network_json) or {
		panic('Failed to decode json for ${filename}, ${err}')
	}

	n.verify_graph() or {
		eprintln(err)
		exit(1)
	}

	mut d := viz.new('n_viz', 'Network', 'blue')
	for i, neuron in n.neurons {
		d.new_node('${i} (${neuron.threshold})', '${i}')
	}

	for i, neuron in n.neurons {
		for s in neuron.pre_synapses {
			d.new_edge('${s.from}', '${i}')
		}
	}

	sixel_graph := os.system("echo 'digraph G {\n${d.sb.str()}}\n}'| dot -Tpng | img2sixel")
	if sixel_graph != 0 {
		panic('Viz not supported on this platform, please try installing `graphviz (dot)` and `img2sixel`')
	}
}

fn run_func(cmd cli.Command) ! {
	filename := cmd.flags.get_string('json') or { panic('Failed to get network json file: ${err}') }

	network_json := os.read_file(filename) or {
		eprintln('Failed to read_file ${filename}, ${err}')
		exit(1)
	}

	mut n := json.decode(network.Network, network_json) or {
		eprintln('Failed to decode json for ${filename}, ${err}')
		exit(1)
	}

	n.verify_graph() or {
		eprintln(err)
		exit(1)
	}

	n.initialize() or {
		eprintln(err)
		exit(1)
	}

	main_loop: for {
		input_line := os.input('Inputs  (${n.input_domain.len}): ')
		if input_line == '<EOF>' {
			println('\n\nGoodbye!')
			exit(0)
		}

		inputs := if input_line.len != 0 {
			input_line.split(' ').map(strconv.atoi(it) or {
				eprintln('Failed to parse ${it} as integer')
				continue main_loop
			})
		} else {
			[]int{}
		}

		if inputs.len != n.input_domain.len && inputs.len != 0 {
			eprintln('Mismatched number of inputs, expected ${n.input_domain.len} got ${inputs.len}')
			continue
		}

		n.input(inputs) or {
			eprintln(err.msg())
			continue
		}

		for _ in 0 .. n.end_to_end_time {
			println(n)
			n.run() or {
				eprintln(err.msg())
				exit(1)
			}
			n.current_timestep++
			n.output()!
		}

		println('Outputs (${n.output_range.len}): ${n.format_output()!}')
		n.reset_output()
	}
}

fn train_classification_func(cmd cli.Command) ! {
	filename := cmd.flags.get_string('dataset') or {
		panic('Failed to get dataset json file: ${err}')
	}

	dataset_json := os.read_file(filename) or {
		eprintln('Failed to read_file ${filename}, ${err}')
		exit(1)
	}

	mut d := json.decode(dataset.Dataset, dataset_json) or {
		eprintln('Failed to decode json for ${filename}, ${err}')
		exit(1)
	}

	d.validate_dataset()!

	initial_network := d.create_skeleton_network()!
	initial_network.verify_graph()!
	println(initial_network)

	mut graph := viz.new('n_viz', 'Network', 'blue')
	for i, neuron in initial_network.neurons {
		graph.new_node('${i} (${neuron.threshold})', '${i}')
	}

	for i, neuron in initial_network.neurons {
		for s in neuron.pre_synapses {
			graph.new_edge('${s.from}', '${i}')
		}
	}

	sixel_graph := os.system("echo 'digraph G {\n${graph.sb.str()}}\n}'| dot -Tpng | img2sixel")
	if sixel_graph != 0 {
		panic('Viz not supported on this platform, please try installing `graphviz (dot)` and `img2sixel`')
	}
}

fn test_classification_func(cmd cli.Command) ! {
	filename := cmd.flags.get_string('dataset') or {
		panic('Failed to get dataset json file: ${err}')
	}

	dataset_json := os.read_file(filename) or {
		eprintln('Failed to read_file ${filename}, ${err}')
		exit(1)
	}

	mut d := json.decode(dataset.Dataset, dataset_json) or {
		eprintln('Failed to decode json for ${filename}, ${err}')
		exit(1)
	}

	d.validate_dataset()!

	network_filename := cmd.flags.get_string('network') or {
		panic('Failed to get network json file: ${err}')
	}

	network_json := os.read_file(network_filename) or {
		eprintln('Failed to read_file ${filename}, ${err}')
		exit(1)
	}

	mut n := json.decode(network.Network, network_json) or {
		eprintln('Failed to decode json for ${filename}, ${err}')
		exit(1)
	}

	n.verify_graph() or {
		eprintln(err)
		exit(1)
	}

	n.initialize() or {
		eprintln(err)
		exit(1)
	}

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

	println('Network passed ${correct_runs}/${d.data.len} tests')
}
