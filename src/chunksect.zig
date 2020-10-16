pub const CHUNK_SECT_MAX_X = 16;
pub const CHUNK_SECT_MAX_Y = 16;
pub const CHUNK_SECT_MAX_Z = 16;

pub const BlockProtocolID = u16;

chunk_x: i32 = 0,
chunk_y: i32 = 0,
chunk_z: i32 = 0,

block_data: [CHUNK_SECT_MAX_X * CHUNK_SECT_MAX_Y * CHUNK_SECT_MAX_Z]BlockProtocolID = undefined,
block_count: u16 = 0,

