module main

// import rand
import arrays
import term
import math

enum Classification {
	padding
	real
}

struct Field {
	size           int
	classification Classification
}

struct Results {
	sizes         []Field
	total_size    int
	total_useful  int
	total_padding int
}

// Keep the fields in the same order,
// align to 4 bytes
fn no_rearrangement(sizes []int) !Results {
	longest_field := arrays.max(sizes)!
	println(longest_field)

	mut output_sizes := []Field{}

	// Loop though 1 "row" of our struct at a time to determine what we can fit
	mut row_size := 0
	for element in sizes {
		// We have exceeded the row length limit
		if row_size + element > longest_field {
			output_sizes << Field{longest_field - row_size, .padding}
			row_size = 0
		}
		// We fit! We must align ourselves to a multiple of our own size
		if row_size % element != 0 {
			padding_needed := math.min(8, element - (row_size % element))
			if row_size + padding_needed > longest_field {
				// We will overflow, just pad out to the end of the line
				output_sizes << Field{longest_field - row_size, .padding}
				row_size = 0
			} else {
				// We have room for padding
				output_sizes << Field{padding_needed, .padding}
				row_size += padding_needed
			}
		}
		// Check if we exceeded line length
		if row_size + element > longest_field {
			output_sizes << Field{longest_field - row_size, .padding}
			row_size = 0
		}

		output_sizes << Field{element, .real}
		row_size += element
	}

	println(row_size)
	if row_size != longest_field {
		output_sizes << Field{longest_field - row_size, .padding}
	}

	output_sizes = output_sizes.filter(it.size != 0)
	print_results(output_sizes, longest_field)

	return Results{
		sizes:         output_sizes
		total_size:    arrays.sum(output_sizes.map(it.size))!
		total_useful:  arrays.sum(output_sizes.filter(it.classification != .padding).map(it.size))!
		total_padding: arrays.sum(output_sizes.filter(it.classification == .padding).map(it.size))!
	}
}

fn sort_and_align(sizes []int) !Results {
	mut copy := sizes.clone()
	copy.sort(a > b)
	println(copy)

	return no_rearrangement(copy)
}

fn print_results(fields []Field, max_line int) {
	// red(str) and blue(str)
	mut line_length := 0
	for field in fields {
		if field.classification == .padding {
			print(term.red('█'.repeat(field.size)))
		} else {
			print(term.blue('█'.repeat(field.size)))
		}

		line_length += field.size
		if line_length == max_line {
			println('')
			line_length = 0
		}
	}
	println('')
}

fn main() {
	// common_sizes := [u32(1), 2, 4, 8, 16, 24, 32, 64]

	// count := rand.u32_in_range(8, 16) or { exit(1) }

	// sizes := rand.sample(common_sizes, count)

	sample := [2, 16, 1, 4, 16, 24, 24, 8]

	println(no_rearrangement(sample)!)

	println(sort_and_align(sample)!)
}
