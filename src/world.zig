const std = @import("std");

const tm = @import("time.zig");
const bus = @import("bus.zig");
usingnamespace @import("events.zig");

const ticksPerDay: u32 = 24000;

const TimeContext = struct{
    time: tm.Time,
    mutex: *std.Mutex
};

var timeMut = std.Mutex{};
pub var timeCtx : TimeContext = TimeContext{
    .mutex = &timeMut,
    .time = tm.Time{}
};

pub fn init() !void {
    try bus.init();
}

pub fn updateTime() !void {
    const timeHeld = timeCtx.mutex.acquire();
    timeCtx.time.timeOfDay = (timeCtx.time.timeOfDay + 1) % ticksPerDay;
    timeCtx.time.worldAge += 1;

    const timeEvent = try std.heap.page_allocator.create(EventTimeUpdate);
    try bus.addEvent(@ptrCast(*Event, timeEvent));
    timeEvent.worldAge = timeCtx.time.worldAge;
    timeEvent.timeOfDay = timeCtx.time.timeOfDay;

    std.debug.warn("{}\n", .{timeEvent.timeOfDay});

    timeHeld.release();
}

pub fn tickUpdate(context: void) !void {
    while(true){
        try updateTime();

        try bus.pushEvents();
    }
}
