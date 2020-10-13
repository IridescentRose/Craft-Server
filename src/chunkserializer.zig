pub const Chunk = @import("chunk.zig");
pub const ChunkSect = @import("chunksect.zig");

//Encode / Decode support
usingnamespace @import("decode.zig");
usingnamespace @import("encode.zig");

const client = @import("client.zig");
const std = @import("std");
pub fn send_chunk(chunk: *Chunk, clnt: *client.Client) !void {
    var buf = try std.heap.page_allocator.alloc(u8, 1024);
    var strm = std.io.fixedBufferStream(buf);
    var writ = strm.writer();

    try writ.writeIntBig(chunk.chunk_x);
    try writ.writeIntBig(chunk.chunk_z);
    try writ.writeByte(1); //Full chunk = true

    var bitmask: u16 = 0;
    try encodeVarInt(bitmask);

    //Heightmaps
    //Biomes
    //Size
    //Array

    //Block Entities are not instantiated here
    try writ.writeByte(0);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x22, clnt.compress);
    defer std.heap.page_allocator.free(buf);
}
