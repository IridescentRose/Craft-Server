//World Time Structure
pub const Time = struct {
    worldAge : i64 = 0,
    timeOfDay : i64 = 0,  
};

//Global time
pub var worldTime : Time = Time{};
