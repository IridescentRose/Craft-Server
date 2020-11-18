const std = @import("std");

const tm = @import("time.zig");
const bus = @import("bus.zig");
usingnamespace @import("events.zig");
usingnamespace @import("difficulty.zig");
usingnamespace @import("gamerules.zig");

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

//Settings for the packet
const DifficultyContext = struct{
    difficulty: DifficultySetting,
    mutex: *std.Mutex
};

var difMut = std.Mutex{};
pub var difficultyCtx : DifficultyContext = DifficultyContext{
    .difficulty = DifficultySetting{
        .difficulty = Difficulty.Easy,
        .locked = false,
    },
    .mutex = &difMut
};


//Current rules
var rulMut = std.Mutex{};
const GameRulesContext = struct{
    rules: GameRules,
    mutex: *std.Mutex
};

pub var rules = GameRulesContext{
    .rules = GameRules{
        .announceAdvancements = true,
        .commandBlockOutput = true, 
        .disableElytraMovementCheck = false,
        .disableRaids = false,
        .doDaylightCycle = true,
        .doEntityDrops = true,
        .doFireTick = true,
        .doInsomnia = true,
        .doImmediateRespawn = false,
        .doLimitedCrafting = false,
        .doMobLoot = true,
        .doMobSpawning = true,
        .doPatrolSpawning = true,
        .doTileDrops = true,
        .doTraderSpawning = true,
        .doWeatherCycle = true,
        .drowningDamage = true,
        .fallDamage = true,
        .fireDamage = true,
        .forgiveDeadPlayers = true,
        .keepInventory = false,
        .logAdminCommands = true,
        .maxCommandChainLength = 65535,
        .maxEntityCramming = 24,
        .mobGriefing = true,
        .naturalRegeneration = true,
        .randomTickSpeed = 3,
        .reducedDebugInfo = false,
        .sendCommandFeedback = true,
        .showCoordinates = true,
        .showDeathMessages = true,
        .spawnRadius = 10,
        .spectatorsGenerateChunks = true,
        .tntExplodes = true,
        .universalAnger = false
    },
    .mutex = &rulMut
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
