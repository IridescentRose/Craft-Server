pub const EventTypes = enum(u16) {
    TimeUpdate = 0,
};

pub const Event = struct {
    etype: EventTypes
};

pub const EventTimeUpdate = struct {
    usingnamespace Event;
    timeOfDay: u64,
    worldAge: u64
};
