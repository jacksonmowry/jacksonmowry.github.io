// Jackson Mowry
// Mon Apr  7 15:13:09 2025
use std::cmp::Ordering;
use std::env::args;
use std::fs::File;
use std::io::Read;
use std::io::Write;

fn main() {
    let args = args();
    let argv: Vec<String> = args.collect();

    if argv.len() != 3 {
        println!("usage: {} input_file output_file", argv[0]);
    }

    let input_file = argv[1].clone();
    let output_file = argv[2].clone();

    let mut file = File::open(input_file).expect("could not open file for reading");

    let mut cvs: Vec<CompValue> = vec![];

    loop {
        let mut buf: [u8; 5] = [0; 5];
        let res = file.read_exact(&mut buf);
        if res.is_err() {
            // We prob hit oef
            break;
        }

        let be_bytes = <[u8; 4]>::try_from(&buf[1..]).unwrap();
        let val = u32::from_be_bytes(be_bytes);

        if buf[0] == 0 {
            cvs.push(CompValue::Bcd(val));
        } else if buf[0] == 1 {
            cvs.push(CompValue::Dpd(val));
        } else {
            panic!("uh oh");
        }
    }

    cvs.sort();

    let mut output = File::create(output_file).expect("Couldn't open output file for writing");

    cvs.iter()
        .for_each(|x| writeln!(&mut output, "{}", x.decode()).expect("Failed to write to file"));
}

enum CompValue {
    Bcd(u32),
    Dpd(u32),
}

impl CompValue {
    pub fn decode(&self) -> u32 {
        match self {
            CompValue::Bcd(val) => decode_bcd(*val),
            CompValue::Dpd(val) => decode_dpd(*val),
        }
    }
}

impl Ord for CompValue {
    fn cmp(&self, other: &Self) -> Ordering {
        self.decode().cmp(&other.decode())
    }
}

impl PartialOrd for CompValue {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl PartialEq for CompValue {
    fn eq(&self, other: &Self) -> bool {
        self.decode() == other.decode()
    }
}

impl Eq for CompValue {}

fn decode_bcd(val: u32) -> u32 {
    val.to_be_bytes()
        .iter()
        .map(|b| (10 * (0xf & (b >> 4))) + (b & 0xf))
        .fold(0, |acc, val| (100 * acc) + val as u32)
}

fn decode_dpd(val: u32) -> u32 {
    1000000 * decode_10bit((val >> 20) as u16 & 0x3FF) as u32
        + 1000 * decode_10bit((val >> 10) as u16 & 0x3FF) as u32
        + decode_10bit(val as u16 & 0x3FF) as u32
}

fn decode_10bit(val: u16) -> u16 {
    let lower = (val >> 1) & 0b111;
    let upper = (val >> 5) & 0b11;
    let that_one_weird_case = !(val >> 3) & 0b1;

    match lower {
        0b111 => match upper {
            0b00 => {
                100 * (8 | ((val >> 7) & 0b1))
                    + 10 * (8 | ((val >> 4) & 0b1))
                    + (((val >> 7) & 0b110) | (val & 0b1))
            }
            0b01 => {
                100 * (8 | ((val >> 7) & 0b1))
                    + 10 * (((val >> 7) & 0b110) | ((val >> 4) & 0b1))
                    + (8 | (val & 0b1))
            }
            0b10 => 100 * ((val >> 7) & 0b111) + 10 * (8 | ((val >> 4) & 0b1)) + (8 | (val & 0b1)),
            0b11 => {
                100 * (8 | ((val >> 7) & 0b1)) + 10 * (8 | ((val >> 4) & 0b1)) + (8 | (val & 0b1))
            }
            _ => unreachable!("Shouldn't get here in upper"),
        },
        0b110 => {
            100 * (8 | ((val >> 7) & 0b1))
                + 10 * ((val >> 4) & 0b111)
                + (((val >> 7) & 0b110) | (val & 0b1))
        }
        0b101 => {
            100 * ((val >> 7) & 0b111)
                + 10 * (8 | (val >> 4) & 0b1)
                + (((val >> 4) & 0b110) | (val & 0b1))
        }
        0b100 => 100 * ((val >> 7) & 0b111) + 10 * ((val >> 4) & 0b111) + (8 | (val & 0b1)),
        _ => {
            if that_one_weird_case == 1 {
                100 * ((val >> 7) & 0b111) + 10 * ((val >> 4) & 0b111) + (val & 0b111)
            } else {
                unreachable!("Shouldn't get here in lower")
            }
        }
    }
}
