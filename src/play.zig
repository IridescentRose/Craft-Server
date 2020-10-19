//Common includes
const std = @import("std");
const log = @import("log");
const packet = @import("packet.zig");
const client = @import("client.zig");
const config = @import("config.zig");
pub const send_chunk = @import("chunkSerializer.zig").send_chunk;
pub const send_light = @import("chunkSerializer.zig").send_light;

//Encode / Decode support
usingnamespace @import("decode.zig");
usingnamespace @import("encode.zig");

const gm = @import("gamemode.zig");
const game = @import("gamerules.zig");

const chat = @import("chat.zig");

//Packet Output Types (remappable for protocols above 578)
pub const PacketTypeOut = enum(u8) {
    ServerDifficulty = 0x0E,
    ChatMessage = 0x0F,
    PluginMessage = 0x19,
    EntityStatus = 0x1C,
    KeepAlive = 0x21,
    JoinGame = 0x26,
    PlayerAbilities = 0x32,
    PlayerPositionLook = 0x36,
    WorldBorder = 0x3E,
    HeldItemChange = 0x40,
    SpawnPosition = 0x4E,
    TimeUpdate = 0x4F,
};

pub const PacketTypeIn = enum(u8) {
    TeleportConfirm = 0x00,
    QueryBlockNBT = 0x01,
    SetDifficulty = 0x02,
    ChatMessage = 0x03,
    ClientStatus = 0x04,
    ClientSettings = 0x05,
    TabComplete = 0x06,
    WindowConfirmation = 0x07,
    ClickWindowButton = 0x08,
    ClickWindow = 0x09,
    CloseWindow = 0x0A,
    PluginMessage = 0x0B,
    EditBook = 0x0C,
    QueryEntityNBT = 0x0D,
    InteractEntity = 0x0E,
    KeepAlive = 0x0F,
    LockDifficulty = 0x10,
    PlayerPosition = 0x11,
    PlayerPositionAndRotation = 0x12,
    PlayerRotation = 0x13,
    PlayerMovement = 0x14,
    VehicleMove = 0x15,
    SteerBoat = 0x16,
    PickItem = 0x17,
    CraftRecipeRequest = 0x18,
    PlayerAbilities = 0x19,
    PlayerDigging = 0x1A,
    EntityAction = 0x1B,
    SteerVehicle = 0x1C,
    RecipeBookData = 0x1D,
    NameItem = 0x1E,
    ResourcePackStatus = 0x1F,
    AdvancementTab = 0x20,
    SelectTrade = 0x21,
    SetBeaconEffect = 0x22,
    HeldItemChange = 0x23,
    UpdateCommandBlock = 0x24,
    UpdateCommandBlockMinecart = 0x25,
    CreativeInventoryAction = 0x26,
    UpdateJigsawBlock = 0x27,
    UpdateStructureBlock = 0x28,
    UpdateSign = 0x29,
    Animation = 0x2A,
    Spectate = 0x2B,
    PlayerBlockPlacement = 0x2C,
    UseItem = 0x2D,
};

//No packet handler yet...
pub fn handlePacket(pack: *packet.Packet, clnt: *client.Client) !void {
    //log.err("PLAY RECEIVED - NOT HANDLED", .{});
    //clnt.shouldClose = true;
    var rd = pack.toStream().reader();
    try rd.skipBytes(1, .{});

    switch (@intToEnum(PacketTypeIn, pack.id)) {
        .KeepAlive => {
            try handle_keep_alive(rd, clnt);
        },

        .ChatMessage => {
            try handle_chat_packet(rd, clnt);
        },

        .PluginMessage => {
            try handle_plugin_message(rd, clnt);
        },

        .TeleportConfirm => {
            try handle_tp_confirm(rd, clnt);
        },

        .PlayerPosition => {
            try handle_player_position(rd, clnt);
        },

        .PlayerPositionAndRotation => {
            try handle_player_position_rotation(rd, clnt);
        },
        
        .PlayerRotation => {
            try handle_player_rotation(rd, clnt);
        },
        
        .PlayerMovement => {
            try handle_player_movement(rd, clnt);
        },

        else => {
            try handle_dummy(pack, clnt);
        },
    }
}

//Handles keep alive - does an error print if it's the wrong ID
pub fn handle_keep_alive(rd: anytype, clnt: *client.Client) !void {
    var id = try rd.readIntBig(u64);
    if (id != 1337) {
        log.err("BAD KEEPALIVE {}", .{id});
    }
}

//TODO: FINISH THIS
pub fn handle_tp_confirm(rd: anytype, clnt: *client.Client) !void {
    var int = try decodeVarInt(rd);
    log.debug("TODO: Implement TP confirm system. ID {}", .{int});
}

//Player Position
pub fn handle_player_position(rd: anytype, clnt: *client.Client) !void{
    @setRuntimeSafety(false);
    var ix = try rd.readIntBig(i64);
    var iy = try rd.readIntBig(i64);
    var iz = try rd.readIntBig(i64);
    var b = try rd.readByte();

    var x = @bitCast(f64, ix);
    var y = @bitCast(f64, iy);
    var z = @bitCast(f64, iz);
    var onGround = b == 1;

    clnt.player.pos.x = x;
    clnt.player.pos.y = y;
    clnt.player.pos.z = z;
    clnt.player.pos.onGround = onGround;
}
//Player Position
pub fn handle_player_position_rotation(rd: anytype, clnt: *client.Client) !void{
    @setRuntimeSafety(false);
    var ix = try rd.readIntBig(i64);
    var iy = try rd.readIntBig(i64);
    var iz = try rd.readIntBig(i64);
    var iyaw = try rd.readIntBig(i32);
    var ipit = try rd.readIntBig(i32);
    var b = try rd.readByte();

    var x = @bitCast(f64, ix);
    var y = @bitCast(f64, iy);
    var z = @bitCast(f64, iz);
    var yaw = @bitCast(f32, iyaw);
    var pitch = @bitCast(f32, ipit);
    var onGround = b == 1;

    clnt.player.pos.x = x;
    clnt.player.pos.y = y;
    clnt.player.pos.z = z;
    clnt.player.pos.yaw = yaw;
    clnt.player.pos.pitch = pitch;
    clnt.player.pos.onGround = onGround;
}
//Player Position
pub fn handle_player_rotation(rd: anytype, clnt: *client.Client) !void{
    @setRuntimeSafety(false);
    var iyaw = try rd.readIntBig(i32);
    var ipit = try rd.readIntBig(i32);
    var b = try rd.readByte();

    var yaw = @bitCast(f32, iyaw);
    var pitch = @bitCast(f32, ipit);
    var onGround = b == 1;

    clnt.player.pos.yaw = yaw;
    clnt.player.pos.pitch = pitch;
    clnt.player.pos.onGround = onGround;
}

//Player Position
pub fn handle_player_movement(rd: anytype, clnt: *client.Client) !void{
    @setRuntimeSafety(false);
    var b = try rd.readByte();
    var onGround = b == 1;
    clnt.player.pos.onGround = onGround;
}

//Output the chat Json
pub fn handle_chat_packet(rd: anytype, clnt: *client.Client) !void {
    var str = try decodeUTF8Str(rd);

    if (str[0] == '/') {
        log.warn("Command Requested - Subsystem does not exist", .{});
        try send_chat_packet(clnt, chat.Text{ .text = "[Server] Found Command - Requested Subsystem Does Not Exist.", .color = "dark_red" });
    } else {
        //Send chat packet
        var text = try std.mem.concat(std.heap.page_allocator, u8, &[_][]const u8{
            clnt.player.username,
            ": ",
            str,
        });
        log.info("{}", .{text});
        var c = chat.Text{ .text = text, .color = "white" };
        try send_chat_packet(clnt, c);
        std.heap.page_allocator.free(text);
    }
}

//Send message in log for plugin message. We don't use these.
pub fn handle_plugin_message(rd: anytype, clnt: *client.Client) !void {
    var str = try decodeUTF8Str(rd);
    log.trace("Received plugin message: {}", .{str});
}

//Dummy handler for what we haven't yet handled!
pub fn handle_dummy(pack: *packet.Packet, clnt: *client.Client) !void {
    log.warn("RECIEVED UNKNOWN PACKET {}", .{pack.id});
    log.warn("ENUM TYPE {}", .{@intToEnum(PacketTypeIn, pack.id)});
}

//Chat packet
pub fn send_chat_packet(clnt: *client.Client, cht: chat.Text) !void {
    var buff2: [1024]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buff2);
    var writ = strm.writer();

    try encodeJSONStr(writ, cht);
    try writ.writeByte(0);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.ChatMessage), clnt.compress);
}

//Sends a Join Game packet for the current player and world
pub fn send_join_game(clnt: *client.Client, eid: i32, gamemode: gm.GameMode, dimension: i32, hashseed: u64, lvlType: []const u8, viewDist: u8) !void {
    var buff2: [128]u8 = undefined;
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
    var buff: [128]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buff);
    var writ = strm.writer();

    try encodeUTF8Str(writ, channel);
    try encodeUTF8Str(writ, data);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.PluginMessage), clnt.compress);
}

const diff = @import("difficulty.zig");
//Sends the current server difficulty
pub fn send_server_difficulty(clnt: *client.Client) !void {
    var buf: [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeByte(@enumToInt(diff.setting.difficulty));
    try writ.writeByte(@boolToInt(diff.setting.locked));
    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.ServerDifficulty), clnt.compress);
}

//Sends the player abilities
pub fn send_player_abilities(clnt: *client.Client) !void {
    var buf: [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeByte(clnt.player.ability.flags);
    try writ.writeIntBig(u32, @bitCast(u32, clnt.player.ability.speed));
    try writ.writeIntBig(u32, @bitCast(u32, clnt.player.ability.fov));

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.PlayerAbilities), clnt.compress);
}

pub fn send_world_border(clnt: *client.Client, x: f64, z: f64, od: f64, nd: f64, speed: usize, portalTPB: usize, warn: usize, warnBlk: usize) !void {
    var buf: [128]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeByte(3);

    try writ.writeIntBig(i64, @bitCast(i64, x));
    try writ.writeIntBig(i64, @bitCast(i64, z));

    try writ.writeIntBig(i64, @bitCast(i64, od));
    try writ.writeIntBig(i64, @bitCast(i64, nd));

    try encodeVarInt(writ, speed);
    try encodeVarInt(writ, portalTPB);

    try encodeVarInt(writ, warn);
    try encodeVarInt(writ, warnBlk);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.WorldBorder), clnt.compress);
}


//Sends the held item change
pub fn send_held_item_change(clnt: *client.Client) !void {
    var buf: [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeByte(clnt.player.slot);
    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.HeldItemChange), clnt.compress);
}

//Sends an entity status
pub fn send_set_entity_status(clnt: *client.Client, eid: i32, status: u8) !void {
    var buf: [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeIntBig(i32, eid);
    try writ.writeByte(status);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.EntityStatus), clnt.compress);
}

//Sends a player position & look with TP ID
pub fn send_player_position_look(clnt: *client.Client, x: f64, y: f64, z: f64, yaw: f32, pitch: f32, flags: u8, tpID: usize) !void {
    var buf: [40]u8 = undefined;
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

const world = @import("world.zig");
//Sends current world time
pub fn send_time_update(clnt: *client.Client) !void {
    var buf: [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    const held = world.timeCtx.mutex.acquire();
    try writ.writeIntBig(u64, world.timeCtx.time.worldAge);
    try writ.writeIntBig(u64, world.timeCtx.time.timeOfDay);
    held.release();

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.TimeUpdate), clnt.compress);
}

//Sends world time with given data.
pub fn send_time_update_e(clnt: *client.Client, worldAge: u64, timeOfDay: u64) !void {
    var buf: [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeIntBig(u64, worldAge);
    try writ.writeIntBig(u64, timeOfDay);

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.TimeUpdate), clnt.compress);
}

//Sends default spawn position of 0, 63, 0
pub fn send_spawn_position(clnt: *client.Client) !void {
    var buf: [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeIntBig(u64, @import("position.zig").getPosition(0, 63, 0));

    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.SpawnPosition), clnt.compress);
}

//Send keep alive with ID
pub fn send_keep_alive(clnt: *client.Client, id: u64) !void {
    var buf: [16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buf);
    var writ = strm.writer();

    try writ.writeIntBig(u64, id);
    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), @enumToInt(PacketTypeOut.KeepAlive), clnt.compress);
}
