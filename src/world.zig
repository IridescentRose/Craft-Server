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
    defer timeHeld.release();
    
    timeCtx.time.timeOfDay = (timeCtx.time.timeOfDay + 1) % ticksPerDay;
    timeCtx.time.worldAge += 1;

    var event = try std.heap.page_allocator.create(Event);
    event.etype = EventTypes.TimeUpdate;

    var eventData = try std.heap.page_allocator.create(tm.Time);
    eventData.* = timeCtx.time;
    event.data = @ptrCast(*EventData, eventData);

    try bus.addEvent(@ptrCast(*Event, event));

}

var oldAge: u64 = 0;
pub fn keepAliveUpdate() !void {
    const timeHeld = timeCtx.mutex.acquire();
    defer timeHeld.release();

    if(timeCtx.time.worldAge / 20 != oldAge){
        oldAge = timeCtx.time.worldAge / 20;

        const keepAliveEvent = try std.heap.page_allocator.create(Event);
        keepAliveEvent.etype = EventTypes.KeepAlive;
        try bus.addEvent(@ptrCast(*Event, keepAliveEvent));
    }
}

pub fn tickUpdate(context: void) !void {
    while(true){
        try updateTime();
        try keepAliveUpdate();

        try bus.pushEvents();
        std.time.sleep(std.time.ns_per_s / 20);
    }
}
