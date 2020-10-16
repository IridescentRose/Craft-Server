
//NBT Tags
pub const TagType = enum(u8){
    End = 0,
    Byte = 1,
    Short = 2,
    Int = 3,
    Long = 4,
    Float = 5,
    Double = 6,
    ByteArray = 7,
    String = 8,
    List = 9,
    Compound = 10,
    IntArray = 11,
    LongArray = 12
};

//Write NBT Tag
//Incomplete
//Data is usually the same as the type
//You must manually write a compound and end tag
//Using a list, the data must be a struct with TagType (ttype) and an Array of said type.
pub fn writeTag(comptime ttype: TagType, comptime name: []const u8, data: anytype, writer: anytype) !void {
    if(ttype == TagType.End){        
        try writer.writeByte(0);
        return;
    }
    
    try writer.writeByte(@enumToInt(ttype));
    try writer.writeIntBig(u16, @truncate(u16, name.len));
    try writer.writeAll(name);

    switch(ttype){
        .Byte => {
            try writer.writeByte(data);
        },
        .Short => {
            try writer.writeIntBig(i16, data);
        },
        .Int => {
            try writer.writeIntBig(i32, data);
        },
        .Long => {
            try writer.writeIntBig(i64, data);
        },
        .Float => {
            try writer.writeIntBig(i32, @bitCast(i32, data));
        },
        .Double => {
            try writer.writeIntBig(i64, @bitCast(i64, data));
        },
        .ByteArray => {
            try writer.writeIntBig(i32, @intCast(i32, data.len));
            try writer.writeAll(data);
        },
        .String => {
            try writer.writeIntBig(u16, @truncate(u16, data.len));
            try writer.writeAll(data);
        },
        .List => {
            try writer.writeIntBig(i32, @intCast(i32, @enumToInt(data.ttype)));
            try writer.writeIntBig(i32, @intCast(i32, data.list.len));
            var i : usize = 0;
            while(i < data.len) : (i += 1){
                try writer.writeIntBig(i32, data.list[i]);
            }
        },
        .IntArray => {
            try writer.writeIntBig(i32, @intCast(i32, data.len));
            var i : usize = 0;
            while(i < data.len) : (i += 1){
                try writer.writeIntBig(i32, data[i]);
            }
        },
        .LongArray => {
            try writer.writeIntBig(i32, @intCast(i32, data.len));
            var i : usize = 0;
            while(i < data.len) : (i += 1){
                try writer.writeIntBig(i64, data[i]);
            }
        },
        .Compound => {
            return;
        },

        else => {
            @compileError("Type not yet implemented!");
        }
    }
}
