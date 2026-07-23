package main

import "core:fmt"
// import "core:io"
import "base:runtime"
import "core:os"
import "vendor:glfw"
import gl "vendor:OpenGL"

framebuffer_size_callback :: proc "c" (window: glfw.WindowHandle, width: i32, height: i32) {
	gl.Viewport(0, 0, width, height)
}

processInput :: proc "c" (window: glfw.WindowHandle) {
	if glfw.GetKey(window, glfw.KEY_ESCAPE) == glfw.PRESS {
		glfw.SetWindowShouldClose(window, true)
	}
}


main :: proc() {
	glfw.Init()
	glfw.WindowHint(glfw.CONTEXT_VERSION_MAJOR, 3)
	glfw.WindowHint(glfw.CONTEXT_VERSION_MINOR, 3)
	glfw.WindowHint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
	window := glfw.CreateWindow(800, 600, "LearnOpenGL", nil, nil)
	if window == nil {
		fmt.println("Failed to create GLFW window")
		glfw.Terminate()
		os.exit(-1)
	}

	glfw.MakeContextCurrent(window)

	gl.load_up_to(3, 3, glfw.gl_set_proc_address)

	gl.Viewport(0, 0, 800, 600)

	glfw.SetFramebufferSizeCallback(window, framebuffer_size_callback)

  vertices := [?]f32 {
    0.5, -0.5, 0.0,  1.0, 0.0, 0.0,
   -0.5, -0.5, 0.0,  0.0, 1.0, 0.0,
    0.0,  0.5, 0.0,  1.0, 0.0, 1.0,
  }

  program_id, loaded_ok := gl.load_shaders_file("vertex_shader.vs", "fragment_shader.fs")
  if !loaded_ok {
    os.exit(-1)
  }

  VBO, VAO: u32
  gl.GenVertexArrays(1, &VAO)
  gl.GenBuffers(1, &VBO)
  gl.BindVertexArray(VAO)

  gl.BindBuffer(gl.ARRAY_BUFFER, VBO)
  gl.BufferData(gl.ARRAY_BUFFER, size_of(vertices), raw_data(&vertices), gl.STATIC_DRAW)
  
  gl.VertexAttribPointer(0, 3, gl.FLOAT, gl.FALSE, 6 * size_of(f32), 0)
  gl.EnableVertexAttribArray(0)

  gl.VertexAttribPointer(1, 3, gl.FLOAT, gl.FALSE, 6 * size_of(f32), 3 * size_of(f32))
  gl.EnableVertexAttribArray(1)

	for !glfw.WindowShouldClose(window) {
		processInput(window)

		gl.ClearColor(0.2, 0.3, 0.3, 1.0)
		gl.Clear(gl.COLOR_BUFFER_BIT)

    gl.UseProgram(program_id)
    gl.BindVertexArray(VAO)
    gl.DrawArrays(gl.TRIANGLES, 0, 3)

		glfw.SwapBuffers(window)
		glfw.PollEvents()
	}

  gl.DeleteVertexArrays(1, &VAO)
  gl.DeleteBuffers(1, &VBO)
  gl.DeleteProgram(program_id)

	glfw.Terminate()
}

// File :: os.File
// read_entire_file :: proc(path: string, allocator: runtime.Allocator) -> ([]u8, bool) {
//   content, err := os.read_entire_file(path, allocator)
//
//   if err != nil {
//     fmt.println("fuck")
//     return {}, false
//   }
//
//   return content, true
// }

//
// file_open :: proc(filename: string) -> (^File, os.Error) {
//   return os.open(filename)
// }
//
// file_read :: proc(f: ^File, p: []byte) -> (n: int, err: os.Error) {
//   return os.read(f, p)
// }
//
// file_seek :: proc(f: ^File, offset: i64, whence: io.Seek_From) -> (ret: i64, err: os.Error) {
//   return os.seek(f, offset, whence)
// }
//
// file_close :: proc(f: ^File) -> os.Error {
//   return os.close(f)
// }

