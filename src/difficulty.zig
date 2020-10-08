pub const Difficulty = enum(u8){
    Peaceful = 0,
    Easy = 1,
    Normal = 2,
    Hard = 3,
};

pub const DifficultySetting = struct{
    difficulty: Difficulty,
    locked: bool,
};

pub var setting : DifficultySetting = DifficultySetting{
    .difficulty = Difficulty.Easy,
    .locked = false,
};
