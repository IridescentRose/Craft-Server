pub const Chunk = @import("chunk.zig");
pub const ChunkSect = @import("chunksect.zig");

//Encode / Decode support
usingnamespace @import("decode.zig");
usingnamespace @import("encode.zig");

const client = @import("client.zig");
const std = @import("std");
const nbt = @import("nbt.zig");

//Generates a bitmask from the chunk section array
pub fn getChunkBitmask(chunk: *Chunk) u16 {
    var res : u16 = 0;

    var y : u5 = 0;
    while(y < 16) : (y += 1){
        if(chunk.chunkList[y] != null){
            var one : u16 = 1;
            res ^= (one << @intCast(u4, y));
        }
    }
    return res;
}

//Gets the number of non-null chunk sections
pub fn getChunkSectCount(chunk: *Chunk) u32 {
    var res : u32 = 0;
    var y : u5 = 0;
    while(y < 16) : (y += 1){
        if(chunk.chunkList[y] != null){
            res += 1;
        }
    }
    return res;
} 

//Serialize a Chunk Section
pub fn serializeChunkSect(strm: anytype, chunk: *ChunkSect) !void{
    @setRuntimeSafety(false);
    var writ = strm.writer();
    try writ.writeIntBig(u16, chunk.block_count);
    
    const bitsperblock = 14;
    try writ.writeByte(bitsperblock); //Global pallete.
    try writ.writeByte(0); //Pallete length = 0

    const csdataarraysize = (16 * 16 * 16 * bitsperblock) / 8 / 8;
    try encodeVarInt(writ, csdataarraysize);

    var tempLong : u64 = 0;
    var currentwritindex : u64 = 0;

    var i: usize = 0;
    while(i < 4096) : (i += 1){
        var value: u64 = chunk.block_data[i];
        const one: u64 = 1;
        var bitmask = (one << bitsperblock) - 1;
        value &= bitmask;
        

        var bitPosition : usize = i * bitsperblock;
        var firstIndex : usize = bitPosition / 64;
        var secondIndex : usize = ((i + 1) * bitsperblock - 1) / 64;
        var bitOffset : usize = bitPosition % 64;

        if(firstIndex != currentwritindex){
            try writ.writeIntBig(u64, tempLong);
            tempLong = 0;
            currentwritindex = firstIndex;
        }

        tempLong |= value << @intCast(u6, bitOffset);

        
        if(firstIndex != secondIndex){
            try writ.writeIntBig(u64, tempLong);
            currentwritindex = secondIndex;
            tempLong = (value >> @intCast(u6, (64 - bitOffset)));
        }
        
    }
    try writ.writeIntBig(u64, tempLong);
}

pub fn send_light(clnt: *client.Client, chunk: *Chunk) !void{
    var buf = try std.heap.page_allocator.alloc(u8, 8192);
    defer std.heap.page_allocator.free(buf);

    var strm = std.io.fixedBufferStream(buf);
    var writ = strm.writer();

    var cx : u32 = @bitCast(u32, chunk.chunk_x);
    if(chunk.chunk_x < 0){
        cx = ~cx;
    }
    var cz : u32 = @bitCast(u32, chunk.chunk_z);
    if(chunk.chunk_z < 0){
        cz = ~cz;
    }
    try encodeVarInt(writ, cx);
    try encodeVarInt(writ, cz);

    //Sk mask
    try encodeVarInt(writ, 2);
    //Bl mask
    try encodeVarInt(writ, 2);

    //ESk mask
    try encodeVarInt(writ, 0);
    //EBl mask
    try encodeVarInt(writ, 0);

    //Sk Arrays
    try encodeVarInt(writ, 2048);
    var i : usize = 0;
    while(i < 2048) : (i += 1){
        try writ.writeByte(0xff);
    }
    
    //Bl Arrays 
    try encodeVarInt(writ, 2048);
    i = 0;
    while(i < 2048) : (i += 1){
        try writ.writeByte(0xff);
    }

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x25, clnt.compress);
}

pub fn send_chunk(clnt: *client.Client, chunk: *Chunk) !void {
    const bitsperblock = 14;
    const csdataarraysize = 16 * 16 * 16 * bitsperblock / 8 / 8;
    const cssize = 1 + 2 + csdataarraysize * 8;
    const chunkSize = cssize * getChunkSectCount(chunk);
    
    var buf = try std.heap.page_allocator.alloc(u8, chunkSize + 4096 + 512);
    defer std.heap.page_allocator.free(buf);
    
    var strm = std.io.fixedBufferStream(buf);
    var writ = strm.writer();

    try writ.writeIntBig(i32, chunk.chunk_x);
    try writ.writeIntBig(i32, chunk.chunk_z);
    try writ.writeByte(1); //Full chunk = true

    var bitmask: u16 = getChunkBitmask(chunk);
    try encodeVarInt(writ, bitmask);

    //Heightmaps
    try nbt.writeTag(nbt.TagType.Compound, "", 0, writ);
    try nbt.writeTag(nbt.TagType.LongArray, "MOTION_BLOCKING", chunk.heightMap, writ);
    try nbt.writeTag(nbt.TagType.End, "", 0, writ);

    //Biomes
    var i: usize = 0;
    while(i < 1024) : (i += 1){
        try writ.writeIntBig(i32, chunk.biomeDesc[i]);
    }
    

    //Array
    var buf2 = try std.heap.page_allocator.alloc(u8, 16384);
    defer std.heap.page_allocator.free(buf2);
    var strm2 = std.io.fixedBufferStream(buf2);
    
    i = 0;
    while(i < 16) : (i += 1){
        if(chunk.chunkList[i] != null){
            try serializeChunkSect(&strm2, chunk.chunkList[i].?);
        }
    }

    
    //Size
    try encodeVarInt(writ, try strm2.getPos());
    
    i = 0;
    while(i < try strm2.getPos()) : (i += 1){
        try writ.writeByte(strm2.getWritten()[i]);
    }

    //Block Entities are not instantiated here
    try writ.writeByte(0);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x22, clnt.compress);
}
