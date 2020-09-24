const std = @import("std");
const log = @import("log");
const network = @import("network");
const Logger = log.Logger;

pub fn main() !void {
    log.init();
    log.info("Craft-Server v0.3", .{});

    try network.init();
    defer network.deinit();

    const sock = try network.connectToHost(std.heap.page_allocator, "tcpbin.com", 4242, .tcp);
    defer sock.close();

    const msg = "Hi from socket!\n";
    try sock.writer().writeAll(msg);

    var buf: [128]u8 = undefined;
    log.info("Echo: {}", .{buf[0..try sock.reader().readAll(buf[0..msg.len])]});
}
