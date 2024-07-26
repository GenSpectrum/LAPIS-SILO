pub fn add1(val: u32) -> Result<u32, &'static str> {
    if val < u32::MAX {
        Ok(val + 1)
    } else {
        Err("add1: u32 number overflow")
    }
}
