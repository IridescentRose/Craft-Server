//Enum for the game difficulty
pub const Difficulty = enum(u8){
    Peaceful = 0,
    Easy = 1,
    Normal = 2,
    Hard = 3,
};

//Structure to determine settings info
pub const DifficultySetting = struct{
    difficulty: Difficulty,
    locked: bool,
};
