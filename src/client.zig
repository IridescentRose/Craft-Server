const std = @import("std");
const log = @import("log");
const network = @import("network");
const netbus = @import("netbus.zig");

const packet = @import("packet.zig");
const server = @import("server.zig");
usingnamespace @import("decode.zig");
usingnamespace @import("encode.zig");

//Minecraft's connection statuses
//Based on what connection status determines the behavior
pub const ConnectionStatus = enum(c_int) {
    pub const Self = @This();

    Handshake = 0,
    Status = 1,
    Login = 2,
    Play = 3,

    //Basic methods
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
    shouldClose: bool,
    loggedIn: bool = false,

    //Read a packet from the reader into an existing buffer.
    pub fn readPacket(reader: anytype, pack: *packet.Packet, compress: bool) !bool{
        var size : u32 = try decodeVarInt(reader);

        //Check the size
        if (size == 0){
            return false;
        }

        pack.buffer = [_]u8{0} ** 512;

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

    //Send a packet from the buffer to the socket.
    pub fn sendPacket(self: *Client, writer: anytype, buf: []u8, id: u8, compress: bool) !void{

        //Allocate our full packet buffer and free it after
        var buffer = try std.heap.page_allocator.alloc(u8, buf.len + 1 + 1 + 5);
        defer std.heap.page_allocator.free(buffer);

        //Make a nice interface
        var strm = std.io.fixedBufferStream(buffer);
        var writ = strm.writer();

        //Encode the VarInt prefix + ID
        try encodeVarInt(writ, buf.len + 1);
        try writ.writeByte(id);

        //Write the rest of the buffer
        var i : usize = 0;
        while(i < buf.len) : (i += 1){
            try writ.writeByte(buf[i]);
        }

        //Write the full buffer to the socket!
        try writer.writeAll(strm.getWritten());
    }

    //Disconnect the player and reduce count.
    pub fn disconnect(self: *Client) !void{
        log.info("Closed connection from {}",.{try self.conn.getLocalEndPoint()});
        if(self.loggedIn){
            server.info.players.online -= 1;
        }
        self.conn.close();
    }

    //Handle our connection object.
    pub fn handle(self: *Client) !void {
        //Basic init
        self.shouldClose = false;
        self.status = ConnectionStatus.Handshake;

        log.info("Client connected from {}", .{
            try self.conn.getLocalEndPoint(),
        });

        //Main loop
        while (!self.shouldClose) {
            //Create a new packet instance
            const reader = self.conn.reader();
            
            const pck = try std.heap.page_allocator.create(packet.Packet);
            defer std.heap.page_allocator.destroy(pck);
            pck.* = packet.Packet{
                .buffer = undefined,
                .size = 0
            };

            //Read packet
            _ = try await async readPacket(reader, pck, self.compress);

            //Handle packet (and send)
            try netbus.handlePacket(pck, self);
        }

        //We were closed - so close out!
        _ = self.disconnect() catch {};
    }
};
