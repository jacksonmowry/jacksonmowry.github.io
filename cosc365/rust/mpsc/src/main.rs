use std::env::args;
use std::sync::mpsc::channel;
use std::thread;

fn main() {
    let a: Vec<String> = args().collect();
    if a.len() != 3 {
        println!("Usage: {} <start> <finish>", a[0]);
        return;
    }

    let start = a[1].parse::<usize>().expect("invalid starting input");
    let end = a[2].parse::<usize>().expect("invalid ending input");

    let (tx, rx) = channel();
    (start..=end).for_each(|i| {
        let mytx = tx.clone();
        thread::spawn(move || {
            if is_prime(i) {
                mytx.send(i).unwrap();
            }
        });
    });

    drop(tx);
    let mut values = vec![];

    while let Ok(val) = rx.recv() {
        values.push(val);
    }

    values.sort();

    values.iter().for_each(|x| println!("{}", x));
}

fn is_prime(val: usize) -> bool {
    let end = f64::sqrt(val as f64) as usize;
    !(2..end).any(|x| val % x == 0)
}
