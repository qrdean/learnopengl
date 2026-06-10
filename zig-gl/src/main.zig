const std = @import("std");
// const Io = std.Io;
const glfw = @import("zglfw");
const zopengl = @import("zopengl");
const zig_gl = @import("zig_gl");

const gl_version_major: u16 = 3;
const gl_version_minor: u16 = 3;

fn glfwErrorCallback(code: glfw.ErrorCode, desc: ?[*:0]const u8) callconv(.c) void {
    if (desc) |d| std.debug.print("GLFW error (0x{x}): {s}\n", .{ @as(c_int, code), d });
}

pub fn main() !void {
    _ = glfw.setErrorCallback(glfwErrorCallback);
    try glfw.init();
    defer glfw.terminate();

    glfw.windowHint(.client_api, .opengl_api);
    glfw.windowHint(.context_version_major, gl_version_major);
    glfw.windowHint(.context_version_minor, gl_version_minor);
    glfw.windowHint(.opengl_profile, .opengl_core_profile);

    const window = try glfw.createWindow(800, 600, "LearnOpenGL", null, null);
    defer window.destroy();

    glfw.makeContextCurrent(window);
    glfw.swapInterval(1);

    try zopengl.loadCoreProfile(glfw.getProcAddress, gl_version_major, gl_version_minor);

    while (!window.shouldClose()) {
        glfw.pollEvents();

        const gl = zopengl.bindings;

        gl.clearColor(0.12, 0.24, 0.36, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT);

        window.swapBuffers();
    }
}

