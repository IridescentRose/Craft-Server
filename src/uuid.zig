//Basic UUID generation
const std = @import("std");

const chars : []const u8 = "0123456789ABCDEF";

pub const UUID = struct{
    const Self = @This();
    id: [36]u8,

    pub fn new(seed: u64) !*Self{
        var r = std.rand.DefaultPrng.init(seed);
        var uu = try std.heap.page_allocator.create(Self);

        var i : usize = 0;
        while(i < 36) : (i += 1){
            uu.id[i] = '0';
        }

        uu.id[8] = '-';
        uu.id[13] = '-';
        uu.id[14] = '4';
        uu.id[18] = '-';
        uu.id[23] = '-';

        i = 0;
        while(i < 36) : (i += 1){
            if(i != 8 and i != 13 and i != 14 and i != 18 and i != 23){
                var res : u8 = r.random.uintLessThanBiased(u8, 16);
                uu.id[i] = chars[res];
            }
        }
        
        return uu;
    }
};
