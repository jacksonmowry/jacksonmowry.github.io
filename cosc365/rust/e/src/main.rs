#[derive(Debug)]
pub enum MyResult<O, E> {
    Ok(O),
    Err(E),
}

impl<O, E> MyResult<O, E> {
    pub fn unwrap(self) -> O {
        match self {
            MyResult::Ok(x) => x,
            MyResult::Err(_) => panic!("unwrap on Err"),
        }
    }

    pub fn expect(self, msg: &str) -> O {
        match self {
            MyResult::Ok(x) => x,
            MyResult::Err(_) => panic!("{}", msg),
        }
    }

    pub fn deref(&self) -> Option<&O> {
        match self {
            MyResult::Ok(x) => Some(x),
            _ => None,
        }
    }
}

fn main() {
    let e: MyResult<i32, &str> = MyResult::Err("a string");
    println!("{}", e.expect("Oh no"));
}
