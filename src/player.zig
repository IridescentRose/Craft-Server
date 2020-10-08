//Basic Player Class
pub const Player = struct{
    username: []const u8,
    uuid: *@import("uuid.zig").UUID,
    ability: Abilities,
    slot: u8 = 0,
};

//Basic Player Abilities
pub const Abilities = struct {
    flags: u8 = 0, //TODO: Make a bitflags structure
    speed: f32 = 0.05,
    fov: f32 = 0.1,
};
