const std = @import("std");
const log = @import("log");
const network = @import("network");

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

    pub fn handle(self: *Client) !void {
        log.info("Client connected from {}", .{
            try self.conn.getLocalEndPoint(),
        });

        while (true) {
            var buf: [1024]u8 = undefined;
            
            const amt = try self.conn.receive(&buf);
            const msg = buf[0..amt];
            
            log.info("Client wrote: {}", .{msg});
            
            if (amt == 0){
                log.info("Closed connection from {}",.{try self.conn.getLocalEndPoint()});
                self.conn.close();
                break; // We're done, end of connection
            }
        }
    }
};
