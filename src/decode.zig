const std = @import("std");
const log = @import("log");

//A basic VarInt Reader
pub fn decodeVarInt(reader: anytype) !u32{
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

//Read a VarInt prepended string
pub fn decodeUTF8Str(reader: anytype) ![]const u8{
    var size : u32 = try decodeVarInt(reader);

    var buff : []u8 = try std.heap.page_allocator.alloc(u8, size);
    
    var i : usize = 0;
    while(i < size) : (i += 1){
        buff[i] = try reader.readByte();
    }

    return buff;
}
