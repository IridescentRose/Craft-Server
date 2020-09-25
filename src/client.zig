const std = @import("std");
const log = @import("log");
const network = @import("network");
const netbus = @import("netbus.zig");

const packet = @import("packet.zig");
usingnamespace @import("decode.zig");

//Minecraft's connection statuses
//Based on what connection status determines the behavior
pub const ConnectionStatus = enum(c_int) {
    pub const Self = @This();

    Handshake = 0,
    Status = 1,
    Login = 2,
    Play = 3,

    pub fn toString(self: Self) []const u8 {
        return switch (self) {
            ConnectionStatus.Handshake => "Handshake",
            ConnectionStatus.Status => "Status",
            ConnectionStatus.Login => "Login",
            ConnectionStatus.Play => "Play",
        };
    }

    pub fn toValue(self: Self) c_int {
        return switch (self){
            ConnectionStatus.Handshake => 0,
            ConnectionStatus.Status => 1,
            ConnectionStatus.Login => 2,
            ConnectionStatus.Play => 2,
        };
    }
};


//Our main client object - which handles each packet.
pub const Client = struct {
    conn: network.Socket,
    handle_frame: @Frame(Client.handle),
    status: ConnectionStatus,
    compress: bool,
    protocolVer: u32,

    //Read a packet from the reader into an existing buffer.
    pub fn readPacket(reader: anytype, pack: *packet.Packet, compress: bool) !bool{
        var size : u32 = try decodeVarInt(reader);

        //Check the size
        if (size == 0){
            return false;
        }

        pack.buffer = [_]u8{0} ** 1024;

        //Read into buffer
        var i : usize = 0;
        while(i < size) : (i += 1){
            pack.buffer[i] = try reader.readByte();
        }
        pack.size = size;

        if(compress){
            log.fatal("COMPRESSION DISABLED!", .{});
        }else{
            pack.id = try pack.toStream().reader().readByte();
        }

        return true;
    }

    //Handle our connection object.
    pub fn handle(self: *Client) !void {
        self.status = ConnectionStatus.Handshake;

        log.info("Client connected from {}", .{
            try self.conn.getLocalEndPoint(),
        });

        while (true) {
            const reader = self.conn.reader();
            
            const pck = try std.heap.page_allocator.create(packet.Packet);
            defer std.heap.page_allocator.destroy(pck);
            pck.* = packet.Packet{
                .buffer = undefined,
                .size = 0
            };

            if(!try readPacket(reader, pck, self.compress)){
                log.info("Closed connection from {}",.{try self.conn.getLocalEndPoint()});
                self.conn.close();
                break;
            }

            try netbus.handlePacket(pck, self);
        }
    }
};
