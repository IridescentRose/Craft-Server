//A genuine MC text object
//Includes all formatting
pub const Text = struct{
    text: []const u8,
    bold: bool = false,
    italic: bool = false,
    underlined: bool = false,
    strikethrough: bool = false,
    obfuscated: bool = false,
    color: []const u8 = "gray",
};

//Structures for on click events
pub const ClickEvent = struct{
    action: []const u8,
    value: []const u8,
};

//Actions on click (use toString())
pub const ClickAction = enum{
    const Self = @This();
    OpenUrl,
    RunCommand,
    SuggestCommand,
    ChangePage,

    pub fn toString(self: *Self) []const u8{
        return switch(self){
            .OpenUrl => "open_url",
            .RunCommand => "run_command",
            .SuggestCommand => "suggest_command",
            .ChangePage => "change_page"
        };
    }
};

//Structures 
pub const HoverEvent = struct{
    action: []const u8,
    value: []const u8,
};

pub const HoverAction = enum{
    const Self = @This();
    ShowText,
    ShowItem,
    ShowEntity,

    pub fn toString(self: *Self) []const u8{
        return switch(self){
            .ShowText => "show_text",
            .ShowItem => "show_item",
            .ShowEntity => "show_entity"
        };
    }
};

//Color enum
pub const Color = enum{
    const Self = @This();
    Black = 0,
    DarkBlue = 1,
    DarkGreen = 2,
    DarkCyan = 3,
    DarkRed = 4,
    DarkPurple = 5,
    Gold = 6,
    Gray = 7,
    DarkGray = 8,
    Blue = 9,
    BrightGreen = 10,
    Cyan = 11,
    Red = 12,
    Pink = 13,
    Yellow = 14,
    White = 15,

    pub fn toString(self: *Self) []const u8{
        return switch(self){
            .Black => "black",
            .DarkBlue => "dark_blue",
            .DarkGreen => "dark_green",
            .DarkCyan => "dark_aqua",
            .DarkRed => "dark_red",
            .DarkPurple => "dark_purple",
            .Gold => "gold",
            .Gray => "gray",
            .DarkGray => "dark_gray",
            .Blue => "blue",
            .BrightGreen => "green",
            .Cyan => "aqua",
            .Red => "red",
            .Pink => "light_purple",
            .Yellow => "yellow",
            .White => "white",
        };
    }
};

//Different styles
pub const Style = enum{
    const Self = @This();
    Obfuscated,
    Bold,
    Strikethrough,
    Underline,
    Italic,
    pub fn toString(self: *Self) []const u8{
        return switch(self){
            .Obfuscated => "obfuscated",
            .Bold => "bold",
            .Strikethrough => "strikethrough",
            .Underline => "underline",
            .Italic => "italic",
        };
    }
};
