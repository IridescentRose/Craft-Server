const std = @import("std");
const log = @import("log");
const server = @import("server.zig");

const buffer_size = 1000;

pub fn main() !void {
    log.init();
    log.info("Craft-Server v0.3", .{});

    try server.init();
    defer server.deinit();

    while (server.shouldRun) {
        try server.update();
    }
}

