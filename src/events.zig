pub const EventTypes = enum(u16) {
    TimeUpdate,
    KeepAlive,
    Chat
};

pub const EventData = opaque {};

pub const Event = struct {
    etype: EventTypes,
    data: ?*EventData
};
