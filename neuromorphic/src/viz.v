module viz

import strings

pub struct DotGraph {
pub mut:
	sb strings.Builder
}

pub fn new(name string, label string, color string) DotGraph {
	mut res := DotGraph{
		sb: strings.new_builder(1024)
	}
	res.writeln('\tedge [fontname="Helvetica",fontsize="10",labelfontname="Helvetica",labelfontsize="10",style="solid",color="black"];')
	res.writeln('\tnode [fontname="Helvetica",fontsize="10",style="filled",fontcolor="black",fillcolor="white",color="black",shape="box"];')
	res.writeln('\tratio=0.66;')
	res.writeln('\trankdir="TB";')
	res.writeln('\tcolor="${color}";')
	return res
}

pub fn (mut d DotGraph) writeln(line string) {
	d.sb.writeln(line)
}

pub fn (mut d DotGraph) new_node(label string, name string) {
	d.writeln('\t${name} [shape="circle",label="${label}"];')
}

pub fn (mut d DotGraph) new_edge(source string, target string, color string) {
	d.writeln('\t${source} -> ${target} [color="${color}"];')
}
