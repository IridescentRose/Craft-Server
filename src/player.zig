//Basic Player Class
pub const Player = struct{
    username: []const u8,
    uuid: *@import("uuid.zig").UUID,
    ability: Abilities,
    pos: Position,
    slot: u8 = 0,
};

//Basic Player Abilities
pub const Abilities = struct {
    flags: u8 = 0, //TODO: Make a bitflags structure
    speed: f32 = 0.05,
    fov: f32 = 0.1,
};


pub const Position = struct {
    x: f64 = 0,
    y: f64 = 0,
    z: f64 = 0,
    onGround: bool = true,
    pitch: f32 = 0,
    yaw: f32 = 0,
};
