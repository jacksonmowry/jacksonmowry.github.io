use std::io::stdin;
fn main() {
    let mut s = String::new();

    stdin().read_line(&mut s).unwrap();

    let num = if s.starts_with("0x") || s.starts_with("0X") {
        i32::from_str_radix(&s.trim()[2..], 16).unwrap()
    } else {
        i32::from_str_radix(s.trim(), 10).unwrap()
    };

    println!("{}", num)
}
