const std = @import("std");

pub fn decodeRead(reader: anytype) !u32{
    var result : u32 = 0;

    var bits : u32 = 0;

    var currB : u8 = try reader.readByte();
    while((currB & 0x80) != 0){
        var temp : u32 = (currB & 0x7F);
        result += temp << @intCast(u5, bits);
        bits += 7;

        currB = try reader.readByte();
    }
    var temp : u32 = (currB & 0x7F);
    result += temp << @intCast(u5, bits);

    return result;
}
