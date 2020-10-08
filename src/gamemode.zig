//Basic GameMode structure for modes and hardcore -> int
pub const GameMode = struct{
    const Self = @This();
    mode: Mode,
    hardcore: bool,

    pub fn toInt(self: *const Self) u8 {
        var int = @enumToInt(self.mode);
        if(self.hardcore){
            int |= 0x8;
        }
        return int;
    } 
};

//Enumerated list of Game Modes
pub const Mode =  enum(u8){
    Survival = 0,
    Creative = 1,
    Adventure = 2,
    Spectator = 3,
};
