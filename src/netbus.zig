const std = @import("std");
const log = @import("log");
const packet = @import("packet.zig");
const client = @import("client.zig");

//Initialize the bus
pub fn init() !void{
    
}

//Deinitialize the bus
pub fn deinit() !void{

}

usingnamespace @import("decode.zig");
usingnamespace @import("encode.zig");

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
        log.err("Handshake with invalid ID {x}", .{pack.id});
    }
}

const config = @import("config.zig");

pub fn handleStatus(pack: *packet.Packet, clnt: *client.Client) !void{
    var rd = pack.toStream().reader();
    try rd.skipBytes(1, .{});

    if(pack.id == 0){
        log.trace("Sending Server Status!", .{});
        var buf: []const u8 = "{\"description\":{\"text\":\"" ++ config.motd ++ "\"},\"players\":{\"max\":10,\"online\":0},\"version\":{\"name\":\"1.15.2\",\"protocol\":578}}";

        var buff2 : [256]u8 = undefined;
        var strm = std.io.fixedBufferStream(&buff2);
        var writ = strm.writer();
        try encodeUTF8Str(writ, buf);

        //Well this is a status request - send a status back!
        try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x00, clnt.compress);
    }else if(pack.id == 1){
        log.trace("Sending Pong!", .{});
        var ping : u64 = try rd.readIntBig(u64);

        var buff2 : [8]u8 = undefined;
        var strm = std.io.fixedBufferStream(&buff2);
        var writ = strm.writer();
        try writ.writeIntBig(u64, ping);

        try clnt.sendPacket(clnt.conn.writer(), strm.getWritten(), 0x01, clnt.compress);
        clnt.shouldClose = true;
    }else{
        log.err("Status with invalid ID {x}", .{pack.id});
    }
}

pub fn handlePacket(pack: *packet.Packet, clnt: *client.Client) !void{
    //Switch on state
    switch(clnt.status){
        .Handshake => {
            try handleHandshake(pack, clnt);
        },

        .Status => {
            try handleStatus(pack, clnt);
        },

        .Login => {
            log.err("LOGIN RECEIVED - NOT HANDLED", .{});
        },

        .Play => {
            log.err("PLAY RECEIVED - NOT HANDLED", .{});
        }
    }
}
