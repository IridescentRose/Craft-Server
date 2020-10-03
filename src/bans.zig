const std = @import("std");
const io = std.io;
const mem = std.mem;
const fs = std.fs;
const log = @import("log");
const T = struct{
    bans: [][]const u8,
};

//Check the bans JSON for the username
pub fn isBanned(user: []const u8) !bool{
    var inFile = try fs.cwd().openFile("bans.json", fs.File.OpenFlags{.read = true});
    defer inFile.close();

    var buf: [std.mem.page_size]u8 = undefined;
    const fileSize = try inFile.inStream().readAll(buf[0..]);
    const filestr = buf[0..fileSize];

    
    const options = std.json.ParseOptions{ .allocator = std.heap.page_allocator };
    const r = try std.json.parse(T, &std.json.TokenStream.init(filestr), options);
    defer std.json.parseFree(T, r, options);
    
    var i : usize = 0;
    while(i < r.bans.len) : (i += 1){
        if(std.mem.eql(u8, r.bans[i], user)){
            return true;
        }
    }
    return false;
}
