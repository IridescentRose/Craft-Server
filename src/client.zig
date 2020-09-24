const std = @import("std");
const log = @import("log");
const network = @import("network");

const varint = @import("varint.zig");

pub const ConnectionStatus = enum(c_int) {
    Handshake = 0,
    Status = 1,
    Login = 2,
    Play = 3
};


pub const Client = struct {
    conn: network.Socket,
    handle_frame: @Frame(Client.handle),
    status: ConnectionStatus = ConnectionStatus.Handshake,

    pub fn readPacket(reader: anytype, buf: []u8) !bool{
        var size : u32 = try varint.decodeRead(reader);
        log.info("Packet size: {}", .{size});

        //Check
        if (size == 0){
            return false;
        }

        //Read into buffer
        var i : usize = 0;
        while(i < size) : (i += 1){
            buf[i] = try reader.readByte();
        }

        return true;
    }

    pub fn handle(self: *Client) !void {
        log.info("Client connected from {}", .{
            try self.conn.getLocalEndPoint(),
        });

        while (true) {
            var buf: [1024]u8 = undefined;
            
            const reader = self.conn.reader();
            if(!try readPacket(reader, buf[0..])){
                log.info("Closed connection from {}",.{try self.conn.getLocalEndPoint()});
                self.conn.close();
            }

            
        }
    }
};
