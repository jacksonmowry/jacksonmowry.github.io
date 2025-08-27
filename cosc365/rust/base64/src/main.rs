use std::fs::File;
use std::io;
use std::io::prelude::*;

fn translate(c: u8) -> u8 {
    return match c as char {
        'A'..='Z' => c - ('A' as u8),
        'a'..='z' => c - ('a' as u8) + 26,
        '0'..='9' => c - ('0' as u8) + 52,
        '+' => 62,
        '/' => 63,
        '=' => 0,
        _ => todo!(),
    };
}

fn decode(input: &String) -> Vec<u8> {
    let mut s: Vec<u8> = Vec::new();
    let input_bytes = input.as_bytes();

    for i in 0..(input.len() / 4) {
        let mut word: u32 = 0;

        word |= ((translate(input_bytes[i * 4 + 0]) as u32) & 0b111111) << 18;
        word |= ((translate(input_bytes[i * 4 + 1]) as u32) & 0b111111) << 12;
        word |= ((translate(input_bytes[i * 4 + 2]) as u32) & 0b111111) << 6;
        word |= ((translate(input_bytes[i * 4 + 3]) as u32) & 0b111111) << 0;

        s.push(((word >> 16) & 0xFF) as u8);
        s.push(((word >> 8) & 0xFF) as u8);
        s.push(((word >> 0) & 0xFF) as u8);
    }

    return s;
}

fn main() {
    let mut s = String::new();
    let stdin = io::stdin();
    stdin.read_line(&mut s).expect("Invalid input line");

    let mut file = File::open(s.trim()).unwrap();
    let mut contents = String::new();
    file.read_to_string(&mut contents).unwrap();

    for c in decode(&contents) {
        print!("{}", c as char);
    }

    println!("");
}
