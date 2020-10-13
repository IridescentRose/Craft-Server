//Common includes
const std = @import("std");
const log = @import("log");
const packet = @import("packet.zig");
const client = @import("client.zig");
const config = @import("config.zig");

//Encode / Decode support
usingnamespace @import("decode.zig");
usingnamespace @import("encode.zig");

//Handles the handshake connection status
pub fn handleHandshake(pack: *packet.Packet, clnt: *client.Client) !void{
    //There's only 1 ID - no switch
    var rd = pack.toStream().reader();
    try rd.skipBytes(1, .{});

    if(pack.id == 0){
        var protocol = try decodeVarInt(rd);

        var servaddr = try decodeUTF8Str(rd); //Ignored
        var port : u16 = try rd.readIntBig(u16); //Ignored
        
        var state : u8 = try rd.readIntBig(u8);
        
        //Debug info
        log.trace("Protocol Ver: {}", .{protocol});
        log.trace("Server Addr: {}", .{servaddr});
        log.trace("Port: {}", .{port});
        log.trace("Requested State: {}", .{@intToEnum(client.ConnectionStatus, state).toString()});

        //Verification check... protocol versions must match!
        if(protocol != 578){
            log.err("Connection attempted with incompatible version! Protocol ID {}", .{protocol});
            if(@intToEnum(client.ConnectionStatus, state) == client.ConnectionStatus.Login){
                log.err("Is login request... Disconnecting...", .{});
            }else{
                log.err("Is status request... Continuing...", .{});
            }
        }

        clnt.status = @intToEnum(client.ConnectionStatus, state);
        clnt.protocolVer = protocol;

        std.heap.page_allocator.free(servaddr);
    }else{
        //Anything else is an error!
        log.err("Handshake with invalid ID {x}", .{pack.id});
        clnt.shouldClose = true;
    }
}

const server = @import("server.zig");

//Handle the status request connection state
pub fn handleStatus(pack: *packet.Packet, clnt: *client.Client) !void{
    var rd = pack.toStream().reader();
    try rd.skipBytes(1, .{});

    if(pack.id == 0){
        //Server Status Request Response
        log.trace("Sending Server Status!", .{});
        var buff2 : [256]u8 = undefined;
        var strm = std.io.fixedBufferStream(&buff2);
        var writ = strm.writer();
        try encodeJSONStr(writ, server.info);

        //Well this is a status request - send a status back!
        try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x00, clnt.compress);
    }else if(pack.id == 1){
        //Server Status Ping Response
        log.trace("Sending Pong!", .{});
        var ping : u64 = try rd.readIntBig(u64);

        var buff2 : [8]u8 = undefined;
        var strm = std.io.fixedBufferStream(&buff2);
        var writ = strm.writer();
        try writ.writeIntBig(u64, ping);

        try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x01, clnt.compress);
        //Close our client
        clnt.shouldClose = true;
    }else{
        //Anything else is an error!
        log.err("Status with invalid ID {x}", .{pack.id});
        clnt.shouldClose = true;
    }
}

//Include bans and chat
const banlist = @import("bans.zig");
const chat = @import("chat.zig");

fn sendLoginDisconnect(reason: chat.Text, clnt: *client.Client) !void {
    var buff2 : [512]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buff2);
    var writ = strm.writer();

    try encodeJSONStr(writ, reason);

    //0x00 is DC
    try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x00, clnt.compress);
    clnt.shouldClose = true;
}

fn sendLoginSuccess(clnt: *client.Client) !void {
    var buff2 : [36 + 16]u8 = undefined;
    var strm = std.io.fixedBufferStream(&buff2);
    var writ = strm.writer();
    
    try encodeUTF8Str(writ, clnt.player.uuid.id[0..]);
    try writ.writeByte(0);
    
    //Swap to playstate
    clnt.status = client.ConnectionStatus.Play;

    try await async clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x02, clnt.compress);
}

//Handle the beginning of the login process
pub fn handleLogin(pack: *packet.Packet, clnt: *client.Client) !void{
    var rd = pack.toStream().reader();
    try rd.skipBytes(1, .{});

    if(pack.id == 0){
        var user : []const u8 = try decodeUTF8Str(rd);
        log.info("User Logging in: {}", .{user});

        //Check ban list
        var banned = try banlist.isBanned(user);
        if(banned){
            log.info("User {} is banned!", .{user});
            log.info("Disconnecting!", .{});
            //Send disconnect packet
            try sendLoginDisconnect(chat.Text{.text="You were banned.", .color="dark_red"}, clnt);

        }else{
            //We're good!
            //Increment player counter.
            if(server.info.players.online + 1 < server.info.players.max){
                server.info.players.online += 1;
                clnt.loggedIn = true;
                
                clnt.player.username = user;
                clnt.player.uuid = try @import("uuid.zig").UUID.new(@intCast(u64, std.time.timestamp()));
                
                try sendLoginSuccess(clnt);

                //Trigger play actions
                try postLoginTrigger(pack, clnt);

            }else{
                try sendLoginDisconnect(chat.Text{.text="Too many people trying to connect!", .color="green"}, clnt);
            }
        }
    }
}


const play = @import("play.zig");
const gm = @import("gamemode.zig");

//On login, let's set up the server! 
//TODO: Remove all "magic" numbers
pub fn postLoginTrigger(pack: *packet.Packet, clnt: *client.Client) !void {
    try play.send_join_game(clnt, 0, gm.GameMode{.mode = gm.Mode.Survival, .hardcore = false}, 0, 0, "default", 8);
    try play.send_plugin_channel(clnt, "minecraft:brand", "Craft-Server");
    try play.send_server_difficulty(clnt);
    try play.send_player_abilities(clnt);
    try play.send_held_item_change(clnt);
    try play.send_set_entity_status(clnt, 0, 27);
    try play.send_player_position_look(clnt, 0, 16, 0, 0, 0, 0, 1337);
    try play.send_time_update(clnt);
    try play.send_spawn_position(clnt);
    try play.send_chunk(clnt);
}

//Generic handle all packets
pub fn handlePacket(pack: *packet.Packet, clnt: *client.Client) !void{
    //Switch on state
    switch(clnt.status){
        .Handshake => {
            //Handle handshake
            try handleHandshake(pack, clnt);
        },

        .Status => {
            //Handle status
            try handleStatus(pack, clnt);
        },

        .Login => {
            //Handle login
            try handleLogin(pack, clnt);
        },

        .Play => {
            //Handle play
            try play.handlePacket(pack, clnt);
        }
    }
}
