use std::io;
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    let mut first = String::new();
    let mut second = String::new();
    let stdin = io::stdin();
    stdin.read_line(&mut first).expect("Invalid input line");
    stdin.read_line(&mut second).expect("Invalid input line");

    let how_many: usize = first.trim().parse().unwrap();
    let num_threads: usize = second.trim().parse().unwrap();

    let mut v: Vec<usize> = Vec::with_capacity(how_many);
    for i in 0..=how_many {
        v.push(i);
    }

    let data_mutex = Arc::new(Mutex::new(v));
    let res_mutex = Arc::new(Mutex::new(0));

    let mut threads = Vec::with_capacity(num_threads);

    for _ in 0..num_threads {
        let data_mutex_clone = Arc::clone(&data_mutex);
        let res_mutex_clone = Arc::clone(&res_mutex);

        threads.push(thread::spawn(move || {
            let result = {
                let mut data = data_mutex_clone.lock().unwrap();
                let next = data.iter().fold(0, |acc, x| acc + (x * 2));
                data.push(next);
                next
            };

            *res_mutex_clone.lock().unwrap() += result;
        }));
    }

    let mut data = data_mutex.lock().unwrap();
    let result = data.iter().fold(0, |acc, x| acc + x * 2);
    data.push(result);
    drop(data);
    *res_mutex.lock().unwrap() += result;

    for t in threads {
        t.join().unwrap();
    }

    println!("{}", *res_mutex.lock().unwrap());
}
