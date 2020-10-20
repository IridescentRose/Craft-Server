const std = @import("std");
const time = std.time;
const log = @import("log");
const network = @import("network");
const netbus = @import("netbus.zig");

const playState = @import("play.zig");
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
        return switch (self) {
            ConnectionStatus.Handshake => 0,
            ConnectionStatus.Status => 1,
            ConnectionStatus.Login => 2,
            ConnectionStatus.Play => 2,
        };
    }
};

const pl = @import("player.zig");
usingnamespace @import("events.zig");
const bus = @import("bus.zig");
const tm = @import("time.zig");
const play = @import("play.zig");

//Our main client object - which handles each packet.
pub const Client = struct {
    conn: network.Socket,
    status: ConnectionStatus,
    compress: bool,
    protocolVer: u32,
    shouldClose: bool,
    loggedIn: bool = false,
    player: pl.Player = undefined,
    readLock: std.Mutex = std.Mutex{},
    writeLock: std.Mutex = std.Mutex{},

    pub fn handleEvent(self: *Client, event: *Event) !void {
        if(self.status == ConnectionStatus.Play){
            switch(event.etype) {
                .TimeUpdate => {
                    var data = @ptrCast(*tm.Time, @alignCast(@alignOf(tm.Time), event.data));
                    try play.send_time_update_e(self, data.worldAge, data.timeOfDay);
                },
                .KeepAlive => {
                    try play.send_keep_alive(self, 133742069);
                }
            }
        }
    }

    //Read a packet from the reader into an existing buffer.
    pub fn readPacket(self: *Client, pack: *packet.Packet) !bool {
        var held = self.readLock.acquire();
        defer held.release();

        var rd = self.conn.reader();
        var size: u32 = try decodeVarInt(rd);

        //Check the size
        if (size == 0) {
            return false;
        }

        pack.buffer = [_]u8{0} ** 512;

        //Read into buffer
        var i: usize = 0;
        while (i < size) : (i += 1) {
            pack.buffer[i] = try rd.readByte();
        }
        pack.size = size;

        if (self.compress) {
            log.fatal("COMPRESSION DISABLED!", .{});
        } else {
            pack.id = try pack.toStream().reader().readByte();
        }

        return true;
    }

    //Send a packet from the buffer to the socket.
    pub fn sendPacket(self: *Client, writer: anytype, buf: []u8, id: u8, compress: bool) !void {
        var held = self.writeLock.acquire();
        defer held.release();

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
        var i: usize = 0;
        while (i < buf.len) : (i += 1) {
            try writ.writeByte(buf[i]);
        }

        //Write the full buffer to the socket!
        try writer.writeAll(strm.getWritten());
    }

    //Disconnect the player and reduce count.
    pub fn disconnect(self: *Client) !void {
        log.info("Closed connection from {}", .{try self.conn.getLocalEndPoint()});
        if (self.loggedIn) {
            server.info.players.online -= 1;
        }
        self.conn.close();
        bus.removeListener(self);
    }

    //Handle our connection object.
    pub fn handle(self: *Client) !void {
        //Basic init
        self.shouldClose = false;
        self.status = ConnectionStatus.Handshake;

        log.info("Client connected from {}", .{try self.conn.getLocalEndPoint(),});

        self.player.ability = pl.Abilities{};
        self.player.slot = 0;

        //Main loop
        while (!self.shouldClose) {
            //Create a new packet instance
            const pck = try std.heap.page_allocator.create(packet.Packet);
            defer std.heap.page_allocator.destroy(pck);
            pck.* = packet.Packet{
                .buffer = undefined,
                .size = 0,
            };

            //Read packet
            _ = self.readPacket(pck) catch{
                _ = self.disconnect() catch unreachable;
                return;
            };

            //Handle packet (and send)
            netbus.handlePacket(pck, self)catch{
                _ = self.disconnect() catch unreachable;
                return;
            };
        }

        //We were closed - so close out!
        _ = try self.disconnect();
    }
};
