//Common includes
const std = @import("std");
const log = @import("log");
const packet = @import("packet.zig");
const client = @import("client.zig");
const config = @import("config.zig");

//Encode / Decode support
usingnamespace @import("decode.zig");
usingnamespace @import("encode.zig");

const gm = @import("gamemode.zig");
const game = @import("gamerules.zig");

//Packet Output Types (remappable for protocols above 578)
pub const PacketTypeOut = enum(u8){
    ServerDifficulty = 0x0E,
    PluginMessage = 0x19,
    EntityStatus = 0x1C,
    JoinGame = 0x26,
    PlayerAbilities = 0x32,
    PlayerPositionLook = 0x36,
    HeldItemChange = 0x40,
    SpawnPosition = 0x4E,
    TimeUpdate = 0x4F,
};

//No packet handler yet...
pub fn handlePacket(pack: *packet.Packet, clnt: *client.Client) !void {
    //log.err("PLAY RECEIVED - NOT HANDLED", .{});
    //clnt.shouldClose = true;
}

//Sends a Join Game packet for the current player and world
pub fn send_join_game(clnt: *client.Client, eid: i32, gamemode: gm.GameMode, dimension: i32, hashseed: u64, lvlType: []const u8, viewDist: u8) !void {
    var buff2 : [128]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buff2);
    var writ = strm.writer();

    try writ.writeIntBig(i32, eid);
    try writ.writeIntBig(u8, gamemode.toInt());
    try writ.writeIntBig(i32, dimension);
    try writ.writeIntBig(u64, hashseed);
    try writ.writeByte(0); //Ignored
    try encodeUTF8Str(writ, lvlType);
    try writ.writeIntBig(u8, viewDist);
    try writ.writeByte(@boolToInt(game.rules.reducedDebugInfo));
    try writ.writeByte(@boolToInt(game.rules.doImmediateRespawn));

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.JoinGame), clnt.compress);
}

//Sends a plugin message to the given client
pub fn send_plugin_channel(clnt: *client.Client, channel: []const u8, data: []const u8) !void {
    var buff : [128]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buff);
    var writ = strm.writer();

    try encodeUTF8Str(writ, channel);
    try encodeUTF8Str(writ, data);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.PluginMessage), clnt.compress);
}

const diff = @import("difficulty.zig");
//Sends the current server difficulty
pub fn send_server_difficulty(clnt: *client.Client) !void {
    var buf : [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeByte(@enumToInt(diff.setting.difficulty));
    try writ.writeByte(@boolToInt(diff.setting.locked));
    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.ServerDifficulty), clnt.compress);
}

//Sends the player abilities
pub fn send_player_abilities(clnt: *client.Client) !void {
    var buf : [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeByte(clnt.player.ability.flags);
    try writ.writeIntBig(u32, @bitCast(u32, clnt.player.ability.speed));
    try writ.writeIntBig(u32, @bitCast(u32, clnt.player.ability.fov));

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.PlayerAbilities), clnt.compress);
}

//Sends the held item change
pub fn send_held_item_change(clnt: *client.Client) !void{
    var buf : [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeByte(clnt.player.slot);
    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.HeldItemChange), clnt.compress);
}

//Sends an entity status
pub fn send_set_entity_status(clnt: *client.Client, eid: i32, status: u8) !void {
    var buf : [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeIntBig(i32, eid);
    try writ.writeByte(status);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.EntityStatus), clnt.compress);
}

//Sends a player position & look with TP ID
pub fn send_player_position_look(clnt: *client.Client, x: f64, y: f64, z: f64, yaw: f32, pitch: f32, flags: u8, tpID: usize) !void{
    var buf : [40]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeIntBig(u64, @bitCast(u64, x));
    try writ.writeIntBig(u64, @bitCast(u64, y));
    try writ.writeIntBig(u64, @bitCast(u64, z));
    
    try writ.writeIntBig(u32, @bitCast(u32, yaw));
    try writ.writeIntBig(u32, @bitCast(u32, pitch));

    try writ.writeIntBig(u8, flags);
    try encodeVarInt(writ, tpID);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.PlayerPositionLook), clnt.compress);
}

const time = @import("time.zig");
//Sends current world time
pub fn send_time_update(clnt: *client.Client) !void{
    var buf : [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeIntBig(i64, time.worldTime.worldAge);
    try writ.writeIntBig(i64, time.worldTime.timeOfDay);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.TimeUpdate), clnt.compress);
}

//Sends default spawn position of 0, 63, 0
pub fn send_spawn_position(clnt: *client.Client) !void{
    var buf : [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeIntBig(u64, @import("position.zig").getPosition(0, 63, 0));

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.SpawnPosition), clnt.compress);
}
