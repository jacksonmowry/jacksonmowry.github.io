use std::default::Default;

#[derive(Debug, Copy, Clone)]
pub struct MyStruct {
    a: i32,
    b: i16,
}

impl MyStruct {
    fn new(a: i32, b: i16) -> Self {
        Self { a, b }
    }

    fn get_a(&self) -> i32 {
        self.a
    }

    fn set_a(&mut self, val: i32) {
        self.a = val;
    }
}

impl Default for MyStruct {
    fn default() -> Self {
        Self::new(0, 0)
    }
}

fn main() {
    let s: MyStruct = Default::default();

    println!("{:?}", s);
}
