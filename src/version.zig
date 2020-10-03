//Basic definition of a version
pub const Version = struct{
    name: []const u8,
    protocol: u16
};

//Constant server version
pub const serverVersion = Version{
    .name = "1.15.2",
    .protocol = 578  
};
