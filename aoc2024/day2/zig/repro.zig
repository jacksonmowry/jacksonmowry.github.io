pub fn main() !void {
    const a: i32 = 4;
    const b: i32 = 7;

    const diff = @abs(a - b);
    if ((diff == 0) or (diff > 3)) {
        const c: i32 = 4;
        _ = c;
    } else {
        const c: i32 = 5;
        _ = c;
    }
}
