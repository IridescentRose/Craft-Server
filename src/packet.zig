const std = @import("std");

//Basic Packet structure
pub const Packet = struct{
    const Self = @This();
    buffer: [512]u8 = undefined,
    size: usize = 0,
    id: u8 = 0xFF,
    
    pub fn toStream(self: *Self) std.io.FixedBufferStream([]u8){
        return std.io.fixedBufferStream(&self.buffer);
    }
};
