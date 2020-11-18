const ChunkSect = @import("chunksect.zig");
const std = @import("std");

chunk_x: i32 = 0,
chunk_z: i32 = 0,

chunkList: [16]?*ChunkSect = undefined,
biomeDesc: [1024]i32 = undefined,
heightMap: [36]i64 = undefined,
