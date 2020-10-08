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

pub const PacketTypeOut = enum(u8){
    JoinGame = 0x26,
};


pub fn handlePacket(pack: *packet.Packet, clnt: *client.Client) !void {
    //log.err("PLAY RECEIVED - NOT HANDLED", .{});
    //clnt.shouldClose = true;
}

pub fn send_join_game(pack: *packet.Packet, clnt: *client.Client, eid: i32, gamemode: gm.GameMode, dimension: i32, hashseed: u64, lvlType: []const u8, viewDist: u8) !void {
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

    //0x00 is DC
    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.JoinGame), clnt.compress);
}
