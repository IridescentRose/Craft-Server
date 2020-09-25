const std = @import("std");
const log = @import("log");
const network = @import("network");
const netbus = @import("netbus.zig");

const packet = @import("packet.zig");
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

    pub fn sendPacket(self: *Client, writer: anytype, buf: []u8, id: u8, compress: bool) !void{

        var buffer = try std.heap.page_allocator.alloc(u8, buf.len + 1 + 1 + 5);
        defer std.heap.page_allocator.free(buffer);

        var strm = std.io.fixedBufferStream(buffer);
        var writ = strm.writer();

        try encodeVarInt(writ, buf.len + 1);
        try writ.writeByte(id);

        var i : usize = 0;
        while(i < buf.len) : (i += 1){
            try writ.writeByte(buf[i]);
        }


        try writer.writeAll(strm.getWritten());
    }

    //Handle our connection object.
    pub fn handle(self: *Client) !void {
        self.shouldClose = false;
        self.status = ConnectionStatus.Handshake;

        log.info("Client connected from {}", .{
            try self.conn.getLocalEndPoint(),
        });

        while (!self.shouldClose) {
            const reader = self.conn.reader();
            
            const pck = try std.heap.page_allocator.create(packet.Packet);
            defer std.heap.page_allocator.destroy(pck);

            pck.* = packet.Packet{
                .buffer = undefined,
                .size = 0
            };

            _ = try await async readPacket(reader, pck, self.compress);

            try netbus.handlePacket(pck, self);
        }

        log.info("Closed connection from {}",.{try self.conn.getLocalEndPoint()});
        self.conn.close();
    }
};
