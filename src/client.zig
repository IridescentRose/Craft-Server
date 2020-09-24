const std = @import("std");
const log = @import("log");
const network = @import("network");

const packet = @import("packet.zig");
const varint = @import("varint.zig");

//Minecraft's connection statuses
//Based on what connection status determines the behavior
pub const ConnectionStatus = enum(c_int) {
    Handshake = 0,
    Status = 1,
    Login = 2,
    Play = 3
};


//Our main client object - which handles each packet.
pub const Client = struct {
    conn: network.Socket,
    handle_frame: @Frame(Client.handle),
    status: ConnectionStatus = ConnectionStatus.Handshake,
    compress: bool = false,

    //Read a packet from the reader into an existing buffer.
    pub fn readPacket(reader: anytype, pack: *packet.Packet, compress: bool) !bool{
        var size : u32 = try await async varint.decodeRead(reader);
        log.info("Packet size: {}", .{size});

        //Check the size
        if (size == 0){
            return false;
        }

        pack.buffer = [_]u8{0} ** 1024;

        //Read into buffer
        var i : usize = 0;
        while(i < size) : (i += 1){
            pack.buffer[i] = try await async reader.readByte();
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
        log.info("Client connected from {}", .{
            try self.conn.getLocalEndPoint(),
        });

        while (true) {
            const reader = self.conn.reader();
            
            var pck : packet.Packet = packet.Packet{
                .buffer = undefined,
                .size = 0
            };

            if(!try readPacket(reader, &pck, self.compress)){
                log.info("Closed connection from {}",.{try self.conn.getLocalEndPoint()});
                self.conn.close();
            }

            
   
        }
    }
};
