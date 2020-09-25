//Common imports
const std = @import("std");
const log = @import("log");
const network = @import("network");
const client = @import("client.zig");

//Should be running
pub var shouldRun : bool = false;

//Set Evented IO
pub const io_mode = .evented;

//Create our server socket
pub var sock : network.Socket = undefined;

//Open the network up
//Bind the socket
//Listen for connections
pub fn init() !void {
    shouldRun = true;
    
    try network.init();
    sock = try network.Socket.create(.ipv4, .tcp);
    
    try sock.bindToPort(25565);
    try sock.listen();
    log.info("Listening at {}", .{try sock.getLocalEndPoint()});
}

//Close the socket
//Deinit the network
pub fn deinit() void{
    sock.close();
    network.deinit();
}

//Update connection loop
pub fn update() !void {
    log.info("Waiting for connection", .{});
    
    //TODO: Replace with real allocator
    //Allocate a client
    const cl = try std.heap.page_allocator.create(client.Client);

    //Create a client object
    cl.* = client.Client{
        .conn = try sock.accept(),
        .handle_frame = async client.Client.handle(cl),
        .status = client.ConnectionStatus.Handshake,
        .protocolVer = 0,
        .compress = false,
        .shouldClose = false,
    };

    //Destroy the client when done
    defer std.heap.page_allocator.destroy(cl);
}

