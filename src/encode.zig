const std = @import("std");
const log = @import("log");

//A basic VarInt Writer
pub fn encodeVarInt(writer: anytype, int: usize) !void{
    var intTemp = int;
    while(intTemp & 0b10000000 > 0){
        try writer.writeByte( @truncate(u8, (intTemp & 0xFF) | 0x80));
        intTemp = intTemp >> 7;
    }
    try writer.writeByte(@truncate(u8,intTemp));
}

//Write A VarInt prepended string
pub fn encodeUTF8Str(writer: anytype, str: []const u8) !void{
    try encodeVarInt(writer, str.len);
    
    var i : usize = 0;
    while(i < str.len) : (i += 1){
        try writer.writeByte(str[i]);
    }
}
