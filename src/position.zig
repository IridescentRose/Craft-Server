
pub fn getPosition(x: u64, y: u64, z: u64) u64 {
    return ((x & 0x3FFFFFF) << 38) | ((z & 0x3FFFFFF) << 12) | (y & 0xFFF);
}
