module main

import json
import arrays.parallel
import os
import cli
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

	train_classification_cmd.add_flag(cli.Flag{
		flag:          .int
		required:      false
		name:          'generations'
		abbrev:        'g'
		description:   'Number of generations to train'
		default_value: ['10']
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
	filename := cmd.flags.get_string('json') or {
		eprintln('Failed to get network json file: ${err.msg()}')
		exit(1)
	}

	network_json := os.read_file(filename) or {
		eprintln(err.msg())
		exit(1)
	}

	n := json.decode(network.Network, network_json) or {
		eprintln('Failed to decode json for ${filename}, ${err.msg()}')
		exit(1)
	}

	n.verify_graph() or {
		eprintln(err)
		exit(1)
	}

	n.visualize()!
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

	generations := cmd.flags.get_int('generations') or { 10 }

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

	mut population := d.generate_initial_population(initial_network)
	mut test_results := []dataset.Test_Result{}

	for i in 0 .. generations {
		println('Generation ${i + 1}, testing ${population.len} networks')
		if i != 0 {
			println('I am passing the last 5 networks of ${test_results.map(it.score)} to generate')
			population = d.generate_next_population(test_results#[-5..].clone().map(it.network.clone()))
		}

		test_results = parallel.amap(population, fn [d] (network_to_test network.Network) dataset.Test_Result {
			res := dataset.Test_Result{
				score:   d.test_network(mut network_to_test.clone()) or { 0 }
				network: network_to_test.clone()
			}
			println(res.score)
			return res
		},
			workers: 8
		).sorted(a.score < b.score)
		println(test_results.map(it.score))

		population = test_results#[-5..].clone().map(it.network.clone())
	}

	for result in test_results#[-5..].clone() {
		result.network.visualize()!
		println('${result.score}/${d.data.len}')
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

		// println(n.format_output()!)
		if n.format_output()! == data.output {
			correct_runs++
		}
		n.reset_output()
	}

	println('Network passed ${correct_runs}/${d.data.len} tests')
}
