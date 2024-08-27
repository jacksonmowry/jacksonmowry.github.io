module main

import serkonda7.termtable
import datatypes
import rand
import arrays
import term
import math

enum Classification {
	padding
	real
}

struct Field {
mut:
	size           int
	classification Classification
	r              int
	g              int
	b              int
}

struct Results {
	sizes         []Field
	total_size    int
	total_useful  int
	total_padding int
}

// TODO: Break larger than 8byte datatypes out into 8byte chunks
// Keep the fields in the same order,
// align to 4 bytes
fn no_rearrangement(sizes []int) !Results {
	longest_field := math.min(8, arrays.max(sizes)!)

	mut output_sizes := []Field{}

	// Loop though 1 "row" of our struct at a time to determine what we can fit
	mut row_size := 0
	for element in sizes {
		// We have exceeded the row length limit
		if element < 8 && row_size + element > longest_field {
			output_sizes << Field{longest_field - row_size, .padding, 0, 0, 0}
			row_size = 0
		}
		// We fit! We must align ourselves to a multiple of our own size
		if row_size % math.min(8, element) != 0 {
			padding_needed := math.min(8, element - (row_size % element))
			if row_size + padding_needed > longest_field {
				// We will overflow, just pad out to the end of the line
				output_sizes << Field{longest_field - row_size, .padding, 0, 0, 0}
				row_size = 0
			} else {
				// We have room for padding
				output_sizes << Field{padding_needed, .padding, 0, 0, 0}
				row_size += padding_needed
				row_size %= longest_field
			}
		}
		// Check if we exceeded line length
		if element < 8 && row_size + element > longest_field {
			output_sizes << Field{longest_field - row_size, .padding, 0, 0, 0}
			row_size = 0
		}

		r := rand.int_in_range(0, 255)!
		g := rand.int_in_range(0, 255)!
		b := rand.int_in_range(0, 255)!
		for i := element; i > 0; i -= 8 {
			output_sizes << Field{math.min(8, element), .real, r, g, b}
			row_size += element % longest_field
		}

		row_size %= longest_field
	}

	if row_size % longest_field != 0 {
		output_sizes << Field{longest_field - row_size, .padding, 0, 0, 0}
	}

	output_sizes = output_sizes.filter(it.size != 0)
	print_results(output_sizes, longest_field)

	return Results{
		sizes:        output_sizes
		total_size:   arrays.sum(output_sizes.map(it.size))!
		total_useful: arrays.sum(output_sizes.filter(it.classification != .padding).map(it.size)) or {
			0
		}
		total_padding: arrays.sum(output_sizes.filter(it.classification == .padding).map(it.size)) or {
			0
		}
	}
}

fn sort_and_align(sizes []int) !Results {
	copy := sizes.sorted(a > b)

	return no_rearrangement(copy)
}

fn sort_low_to_high(sizes []int) !Results {
	copy := sizes.sorted(a < b)

	return no_rearrangement(copy)
}

fn best_fit(sizes []int) !Results {
	copy := sizes.sorted(a > b)

	mut layout := []Field{}
	mut total_size := 0

	outer: for field in copy {
		println(layout)
		r := rand.int_in_range(0, 255)!
		g := rand.int_in_range(0, 255)!
		b := rand.int_in_range(0, 255)!
		alignment := math.min(8, field)

		mut pos := 0
		for i, mut val in layout {
			if val.classification != .padding {
				pos += val.size
				continue
			}

			if val.size < field {
				pos += val.size
				continue
			}

			if pos % alignment != 0 {
				// We need to add padding before field
				padding_needed := field - (pos % field)
				if padding_needed + field > val.size {
					pos += val.size
					continue
				}

				val.size = padding_needed
				layout.insert(i + 1, Field{field, .real, r, g, b})
				pos += val.size
				continue outer
			} else {
				// No padding needed
				if field == val.size {
					// Fits exactly
					val = Field{field, .real, r, g, b}
					pos += val.size
					continue outer
				} else {
					// Need to add padding subchunk after field
					padding := val.size - field
					val = Field{field, .real, r, g, b}
					layout.insert(i + 1, Field{padding, .padding, r, g, b})
					pos += val.size
					continue outer
				}
			}
		}

		if total_size % alignment == 0 {
			for size := field; size > 0; size -= 8 {
				layout << Field{math.min(8, size), .real, r, g, b}
			}
			total_size += field
		} else {
			layout << Field{alignment - (total_size % alignment), .padding, 0, 0, 0}
			total_size += alignment - (total_size % alignment)
			for size := field; size > 0; size -= 8 {
				layout << Field{math.min(8, size), .real, r, g, b}
			}
			total_size += field
		}

		if total_size % 8 != 0 {
			layout << Field{total_size % 8, .padding, 0, 0, 0}
			total_size += total_size % 8
		}
	}
	print_results(layout, 8)
	return Results{}
}

fn print_results(fields []Field, max_line int) {
	mut line_length := 0
	for field in fields {
		if field.classification == .padding {
			print(term.white('█'.repeat(field.size)))
		} else {
			print(term.rgb(field.r, field.g, field.b, '█'.repeat(field.size)))
		}

		line_length += field.size
		if line_length % max_line == 0 {
			println('')
		}
	}
	println('')
	println('')
}

struct Stats {
mut:
	total_size    f64
	total_used    f64
	total_padding f64
}

fn main() {
	common_sizes := [1, 2, 4, 6, 8, 16]

	// mut vanilla_stats := Stats{}
	// mut low_high_stats := Stats{}
	// mut high_low_stats := Stats{}

	// runs := 100000

	// for _ in 0 .. runs {
	count := rand.u32_in_range(8, 16) or { exit(1) }

	mut sizes := rand.sample(common_sizes, count)

	sizes << sizes.len / 8 + 1

	// 	one := no_rearrangement(sizes)!
	// 	two := sort_and_align(sizes)!
	// 	three := sort_low_to_high(sizes)!

	// 	vanilla_stats.total_size += one.total_size
	// 	vanilla_stats.total_used += one.total_useful
	// 	vanilla_stats.total_padding += one.total_padding

	// 	low_high_stats.total_size += two.total_size
	// 	low_high_stats.total_used += two.total_useful
	// 	low_high_stats.total_padding += two.total_padding

	// 	high_low_stats.total_size += three.total_size
	// 	high_low_stats.total_used += three.total_useful
	// 	high_low_stats.total_padding += three.total_padding
	// }

	// vanilla_usage_ratio := f64(vanilla_stats.total_used) / vanilla_stats.total_size
	// lh_usage_ratio := f64(low_high_stats.total_used) / low_high_stats.total_size
	// hl_usage_ratio := f64(high_low_stats.total_used) / high_low_stats.total_size

	// data := [
	// 	['Style', 'Total Size', 'Total Used', 'Total Padding', 'Usage Ratio',
	// 		'Ratio difference from Vanilla'],
	// 	['Vanilla', '${vanilla_stats.total_size / runs}', '${vanilla_stats.total_used / runs}',
	// 		'${vanilla_stats.total_padding / runs}', '${vanilla_usage_ratio:.2f}', '1.00'],
	// 	['Low to High', '${low_high_stats.total_size / runs}', '${low_high_stats.total_used / runs}',
	// 		'${low_high_stats.total_padding / runs}', '${lh_usage_ratio:.2f}',
	// 		'${lh_usage_ratio / vanilla_usage_ratio:.2f}'],
	// 	['High to Low', '${high_low_stats.total_size / runs}', '${high_low_stats.total_used / runs}',
	// 		'${high_low_stats.total_padding / runs}', '${hl_usage_ratio:.2f}',
	// 		'${hl_usage_ratio / vanilla_usage_ratio:.2f}'],
	// ]

	// t := termtable.Table{
	// 	data: data
	// }

	// println(t)
	//
	best_fit(sizes)!
}

// sample := [2, 16, 1, 4, 16, 24, 24, 8, 1]
