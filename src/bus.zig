const std = @import("std");
const client = @import("client.zig");
usingnamespace @import("events.zig");

var eventList = std.ArrayList(*Event).init(std.heap.page_allocator);
var eventsMutex = std.Mutex{};

var listenerList = std.ArrayList(*client.Client).init(std.heap.page_allocator);
var listenerMutex = std.Mutex{};

pub fn init() !void {
    eventList.shrink(0);
    listenerList.shrink(0);
}

pub fn addEvent(event: *Event) !void {
    const held = eventsMutex.acquire();
    defer held.release();

    try eventList.append(event);    
}

pub fn addListener(listener: *client.Client) !void {
    const held = listenerMutex.acquire();
    defer held.release();

    try listenerList.append(listener);
}

pub fn removeListener(listener: *client.Client) void {
    const held = eventsMutex.acquire();
    defer held.release();

    const held2 = listenerMutex.acquire();
    defer held2.release();

    var i: usize = 0;
    while(i < listenerList.items.len) : (i += 1){
        if(listenerList.items[i] == listener) {
            _ = listenerList.swapRemove(i);
            return;
        }
    }
}

const chat = @import("chat.zig");

pub fn pushEvents() !void {

    const held = listenerMutex.acquire();
    defer held.release();

    const held2 = eventsMutex.acquire();
    defer held2.release();

    var i: usize = 0;
    while(i < eventList.items.len) : (i += 1){
        var j : usize = 0;
        while(j < listenerList.items.len) : (j += 1){
            try listenerList.items[j].handleEvent(eventList.items[i]);
        }
    }

    i = 0;
    while(i < eventList.items.len) : (i += 1){
        if(eventList.items[i].etype == EventTypes.Chat){
            var data = @ptrCast(*chat.Text, @alignCast(@alignOf(chat.Text), eventList.items[i].data));
            std.heap.page_allocator.free(data.text);
            std.heap.page_allocator.destroy(data);
        }

        std.heap.page_allocator.destroy(eventList.items[i]);
    }
    eventList.shrink(0);
}
