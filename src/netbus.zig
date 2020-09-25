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

    }else{
        log.err("Handshake with invalid ID {}", .{pack.id});
    }
}

pub fn handlePacket(pack: *packet.Packet, clnt: *client.Client) !void{
    //Switch on state
    switch(clnt.status){
        .Handshake => {
            try handleHandshake(pack, clnt);
        },

        .Status => {
            log.err("STATUS RECEIVED - NOT HANDLED", .{});
        },

        .Login => {
            log.err("LOGIN RECEIVED - NOT HANDLED", .{});
        },

        .Play => {
            log.err("PLAY RECEIVED - NOT HANDLED", .{});
        }
    }
}
