
pub const Player = struct{
    username: []const u8,
    uuid: *@import("uuid.zig").UUID,   
};
