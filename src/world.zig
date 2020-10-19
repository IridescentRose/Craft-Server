const std = @import("std");

const tm = @import("time.zig");

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
}

pub fn tickUpdate(context: void) !void {
    while(true){
        
        const timeHeld = timeCtx.mutex.acquire();
        timeCtx.time.timeOfDay = (timeCtx.time.timeOfDay + 1) % ticksPerDay;
        timeCtx.time.worldAge += 1;
        timeHeld.release();

        std.time.sleep(std.time.ns_per_ms * 50);
    }
}
