import os
import arrays
import math

fn main() {
	// input_dimension := os.input('# of Dimension in Input ').int()
	input_dimension := 8

	mut lower := []f64{}
	mut upper := []f64{}
	mut buckets := []int{}
	// for i in 0 .. input_dimension {
	// 	lower << (os.input('Input ${i} lower bound ')).f64()
	// 	upper << (os.input('Input ${i} upper bound ')).f64()
	// 	buckets << (os.input('Input ${i} # buckets ')).f64()
	// }

	file := os.read_file('db.csv')!

	mut entries := file.split('\n').map(it.split(',').map(it.f64())).filter(it.len ==
		input_dimension + 1)

	// for row in entries {
	// 	println(row)
	// }
	for i in 0 .. input_dimension - 1 {
		column_vec := entries.map(it[i])
		lower << arrays.min(column_vec)!
		upper << arrays.max(column_vec)!
		buckets << 10
		// println(lower.last())
		// println(upper.last())
		// println(buckets.last())
	}

	println(lower)
	println(upper)
	println(buckets)

	for mut line in entries {
		for i in 0 .. input_dimension - 1 {
			bucket_size := (upper[i] - lower[i]) / buckets[i]
			line[i] = int(math.floor(f64(line[i] - lower[i]) / f64(bucket_size)))
		}
	}

	int_entries := entries.map(it.map(int(it)))

	for row in int_entries {
		println('{"input": ${row[0..8]}, "output": ["${if row.last() == 1 {
			'diabetes'
		} else {
			'no diabetes'
		}}"]},')
	}
}
