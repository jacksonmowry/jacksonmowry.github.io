use std::io;

fn divide(input: &str) -> (String, String) {
    let len = input.len();

    return (
        input[0..(len / 2)].to_string(),
        input[(len / 2)..].to_string(),
    );
}

fn rotate(input: &String) -> String {
    let mut wrap = 0;
    let mut ret = String::with_capacity(input.len());

    for c in input.chars() {
        wrap += (c as i32) - ('A' as i32);
    }

    for c in input.chars() {
        let add_wrap = ((c as i32) - 'A' as i32) + wrap % 26;
        let add_a = add_wrap % 26 + ('A' as i32);
        ret.push(add_a as u8 as char);
    }

    return ret;
}

fn merge(left: &String, right: &String) -> String {
    let mut ret = String::with_capacity(left.len());

    for (i, c) in left.char_indices() {
        let moved = ((c as i32) - ('A' as i32)) + ((right.as_bytes()[i] as i32) - ('A' as i32));
        let restored = (moved % 26) + ('A' as i32);

        ret.push(restored as u8 as char);
    }

    return ret;
}

fn main() {
    // let (a, b) = divide("EWPGAJRB");
    // let rot_a = rotate(&a);
    // let rot_b = rotate(&b);
    // let f = merge(&rot_a, &rot_b);

    let mut line = String::new();
    let stdin = io::stdin();
    stdin.read_line(&mut line).unwrap();

    if line.trim().len() % 2 != 0 {
        println!("Error: The number of characters is not even!");

        return;
    }

    let (a, b) = divide(line.to_uppercase().as_str());
    let rot_a = rotate(&a);
    let rot_b = rotate(&b);
    let f = merge(&rot_a, &rot_b);

    println!("{}", f);
}
