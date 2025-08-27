use std::fs::File;
use std::io::stdin;
use std::io::{BufRead, BufReader};

fn main() {
    let mut s = String::new();
    stdin().read_line(&mut s).unwrap();

    let f = File::open(s.trim()).expect("unable to open file");
    let br = BufReader::new(f);

    let v: Vec<String> = br.lines().map(|x| x.unwrap()).collect();

    let mut sum = 0;

    for s in v {
        let c = s.chars();
        let mut buf = String::new();

        c.for_each(|x| buf.push(to_num(x)));

        if buf.len() == 0 {
            continue;
        }

        let val: i64 = buf.parse().unwrap();
        sum += val;
        println!("{}", val);
    }

    println!("Sum = {}", sum);
}

fn to_num(c: char) -> char {
    match c {
        'a' => '0',
        'b' => '1',
        'c' => '2',
        'd' => '3',
        'e' => '4',
        'f' => '5',
        'g' => '6',
        'h' => '7',
        'i' => '8',
        'j' => '9',
        _ => '7',
    }
}
