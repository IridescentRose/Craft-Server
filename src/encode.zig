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
    try writer.writeAll(str);
}

//Write A VarInt prepended string from a given struct in JSON format
pub fn encodeJSONStr(writer: anytype, value: anytype) !void{
    var buff2 : [std.mem.page_size]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buff2);
    try std.json.stringify(value, std.json.StringifyOptions{}, strm.outStream());
    try encodeUTF8Str(writer, strm.getWritten());
}
