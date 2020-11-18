const std = @import("std");
const log = @import("log");

//A basic VarInt Writer
pub fn encodeVarInt(writer: anytype, int: usize) !void{
    var intTemp = int;

    //Fixed implementation
    var b: [5]u8 = [_]u8{0} ** 5;
    var idx: usize = 0;

    if(intTemp > 0x7f){
        b[idx] = (@truncate(u8, intTemp) & 0x7f) | 0x80;
    }else{
        b[idx] = (@truncate(u8, intTemp) & 0x7f) | 0x00;
    }
    idx += 1;
    intTemp = intTemp >> 7;

    while(intTemp > 0){
        if(intTemp > 0x7f){
        b[idx] = (@truncate(u8, intTemp) & 0x7f) | 0x80;
        }else{
            b[idx] = (@truncate(u8, intTemp) & 0x7f) | 0x00;
        }
        idx += 1;
        intTemp = intTemp >> 7;
    }

    try writer.writeAll(b[0..idx]);
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
