//Simple multi-platform logger
const std = @import("std");
const builtin = @import("builtin");

const io = std.io;
const os = std.os;
const fs = std.fs;

pub const Level = enum {
    const Self = @This();

    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,

    fn toString(self: Self) []const u8 {
        return switch (self) {
            Level.Trace => "TRACE",
            Level.Debug => "DEBUG",
            Level.Info => "INFO",
            Level.Warn => "WARN",
            Level.Error => "ERROR",
            Level.Fatal => "FATAL",
        };
    }
};

var level: Level = Level.Trace;
var quiet: bool = false;
var start: u64 = 0;

pub fn init() void {
    start = std.time.milliTimestamp();
}

pub fn log(lv: Level, comptime fmt: []const u8, args: var) !void {
    if (@enumToInt(lv) < @enumToInt(level)) {
        return;
    }

    if(!quiet){
        std.debug.warn("[{}] ", .{ std.time.milliTimestamp() - start});
        std.debug.warn("[{}]", .{lv.toString()});
        std.debug.warn(": ", .{});
        std.debug.warn(fmt, args);
        std.debug.warn("\n", .{});
    }
}

pub fn setLevel(lv: Level) void {
    level = lv;
}

pub fn trace(comptime fmt: []const u8, args: var) void {
    log(Level.Trace, fmt, args) catch return;
}
pub fn debug(comptime fmt: []const u8, args: var) void {
    log(Level.Debug, fmt, args) catch return;
}
pub fn info(comptime fmt: []const u8, args: var) void {
    log(Level.Info, fmt, args) catch return;
}
pub fn warn(comptime fmt: []const u8, args: var) void {
    log(Level.Warn, fmt, args) catch return;
}
pub fn err(comptime fmt: []const u8, args: var) void {
    log(Level.Error, fmt, args) catch return;
}
pub fn fatal(comptime fmt: []const u8, args: var) void {
    log(Level.Fatal, fmt, args) catch return;
}
