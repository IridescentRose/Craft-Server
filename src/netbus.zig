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

//Handle the status request connection state
pub fn handleStatus(pack: *packet.Packet, clnt: *client.Client) !void{
    var rd = pack.toStream().reader();
    try rd.skipBytes(1, .{});

    if(pack.id == 0){
        //Server Status Request Response
        log.trace("Sending Server Status!", .{});
        var buf: []const u8 = "{\"description\":{\"text\":\"" ++ config.motd ++ "\"},\"players\":{\"max\":10,\"online\":0},\"version\":{\"name\":\"1.15.2\",\"protocol\":578}}";

        var buff2 : [256]u8 = undefined;
        var strm = std.io.fixedBufferStream(&buff2);
        var writ = strm.writer();
        try encodeUTF8Str(writ, buf);

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

const banlist = @import("bans.zig");
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

            

            clnt.shouldClose = true;
        }
    }
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
            log.err("PLAY RECEIVED - NOT HANDLED", .{});
            clnt.shouldClose = true;
        }
    }
}
