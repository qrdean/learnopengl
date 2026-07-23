package main

import "core:fmt"
import "core:math/linalg"
import "core:os"

import gl "vendor:OpenGL"
import "vendor:glfw"

GL_MAJOR_V :: 3
GL_MINOR_V :: 3
WINDOW_WIDTH :: 800
WINDOW_HEIGHT :: 600

/****** GAME STATE ******/

game_state :: enum {
	ACTIVE,
	MENU,
	WIN,
}

game :: struct {
	state:  game_state,
	width:  u32,
	height: u32,
}

create_game :: proc() -> game {
	return game{state = .ACTIVE, width = WINDOW_WIDTH, height = WINDOW_HEIGHT}
}

destroy_game :: proc(self: game) {}

init :: proc(self: game) {}
update :: proc(self: game, dt: f32) {}
render :: proc(self: game, dt: f32) {}

shader :: struct {
	id: u32,
}

use_shader :: proc(s: shader) -> shader {
	gl.UseProgram(s.id)
	return s
}
compile_shader :: proc(
	self: ^shader,
	vertex_source, fragment_source: ^cstring,
	// geometry_source: string = nil,
) {
	sVertex, sFragment, gShader: u32
	sVertex = gl.CreateShader(gl.VERTEX_SHADER)
	gl.ShaderSource(sVertex, 1, vertex_source, nil)
	gl.CompileShader(sVertex)
	check_compile_errors(sVertex, "VERTEX")
	sFragment = gl.CreateShader(gl.FRAGMENT_SHADER)
	gl.ShaderSource(sFragment, 1, fragment_source, nil)
	gl.CompileShader(sFragment)
	check_compile_errors(sFragment, "FRAGMENT")

	// if (geometry_source != nil) {
	//
	// }

	self.id = gl.CreateProgram()
	gl.AttachShader(self.id, sVertex)
	gl.AttachShader(self.id, sFragment)
	gl.LinkProgram(self.id)
	check_compile_errors(self.id, "PROGRAM")
	gl.DeleteShader(sVertex)
	gl.DeleteShader(sFragment)
}

set_float_shader :: proc(self: shader, name: cstring, val: f32, use: bool = false) {
	if use {
		use_shader(self)
	}
	gl.Uniform1f(gl.GetUniformLocation(self.id, name), val)
}

set_integer_shader :: proc(self: shader, name: cstring, val: i32, use: bool = false) {
	if use {
		use_shader(self)
	}
	gl.Uniform1i(gl.GetUniformLocation(self.id, name), val)
}

set_vector2f_shader :: proc(self: shader, name: cstring, x, y: f32, use: bool = false) {
	if use {
		use_shader(self)
	}
	gl.Uniform2f(gl.GetUniformLocation(self.id, name), x, y)
}

set_vector3f_shader :: proc(self: shader, name: cstring, x, y, z: f32, use: bool = false) {
	if use {
		use_shader(self)
	}
	gl.Uniform3f(gl.GetUniformLocation(self.id, name), x, y, z)
}

set_vector4f_shader :: proc(self: shader, name: cstring, x, y, z, w: f32, use: bool = false) {
	if use {
		use_shader(self)
	}
	gl.Uniform4f(gl.GetUniformLocation(self.id, name), x, y, z, w)
}

set_matrix_shader :: proc(
	self: shader,
	name: cstring,
	matx: linalg.Matrix4x4f32,
	use: bool = false,
) {
	if use {
		use_shader(self)
	}
	m := matx
	gl.UniformMatrix4fv(gl.GetUniformLocation(self.id, name), 1, false, raw_data(&m))
}

check_compile_errors :: proc(object: u32, type: string) {
	success: i32
	info_log: []u8
	if type != "PROGRAM" {
		gl.GetShaderiv(object, gl.COMPILE_STATUS, &success)
		if success != 1 {
			gl.GetShaderInfoLog(object, 1024, nil, raw_data(info_log))
			fmt.printf("| ERROR::SHADER: Compile-time error: Type: %s \n", type)
		}
	} else {
		gl.GetProgramiv(object, gl.LINK_STATUS, &success)
		if success != 1 {
			gl.GetShaderInfoLog(object, 1024, nil, raw_data(info_log))
			fmt.printf("| ERROR::SHADER: Compile-time error: Type: %s \n", type)
		}
	}
}

Texture2D :: struct {
	Id:              u32,
	width:           i32,
	height:          i32,
	internal_format: i32,
	image_format:    u32,
	wrap_s:          i32,
	wrap_t:          i32,
	filter_min:      i32,
	filter_max:      i32,
}

new_texture_2d :: proc() -> Texture2D {
	t := Texture2D {
		width           = 0,
		height          = 0,
		internal_format = gl.RGB,
		image_format    = gl.RGB,
		wrap_s          = gl.REPEAT,
		wrap_t          = gl.REPEAT,
		filter_min      = gl.LINEAR,
		filter_max      = gl.LINEAR,
	}
	gl.GenTextures(1, &t.Id)
	return t
}

generate_texture_2d :: proc(self: ^Texture2D, width, height: i32, data: rawptr) {
	self.width = width
	self.height = height
	// create texture
	gl.BindTexture(gl.TEXTURE_2D, self.Id)
	gl.TexImage2D(
		gl.TEXTURE_2D,
		0,
		self.internal_format,
		width,
		height,
		0,
		self.image_format,
		gl.UNSIGNED_BYTE,
		data,
	)
	// set texture wrap and filters
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, self.wrap_s)
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, self.wrap_t)
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, self.filter_min)
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, self.filter_max)
	// unbind texture
	gl.BindTexture(gl.TEXTURE_2D, 0)
}

bind_texture :: proc(self: ^Texture2D) {
	gl.BindTexture(gl.TEXTURE_2D, self.Id)
}

resource_manager :: struct {
	shaders:  map[string]shader,
	textures: map[string]Texture2D,
}

new_resource_manager :: proc() -> resource_manager {
  return resource_manager{}
}

load_shader :: proc(rm: resource_manager, vShaderFile, fShaderFile, gShaderFile, name: string) -> shader {
  rm.shaders[name] = load_shader_from_file(vShaderFile, fShaderFile, gShaderFile)
  return rm.shaders[name]
}

get_shader :: proc(rm: resource_manager, name: string) -> shader {
  return shaders[name]
}

load_texture :: proc(rm: resource_manager, file: string, alpha: bool, name: string) -> Texture2D { 
  rm.textures[name] = load_texture_from_file(file, alpha)
  return rm.textures[name]
}
get_texture :: proc(rm: resource_manager, name: string) -> Texture2D {
  return rm.textures[name]
}
clear_resource_manager :: proc(rm: resource_manager) {
  for n in rm.shaders {
    gl.DeleteProgram(rm.shaders[n].id)
  }

  for n in rm.textures {
    t := rm.textures[n]
    gl.DeleteTextures(1, &t.Id)
  }
}
load_shader_from_file :: proc(vShaderFile, fShaderFile, gShaderFile: string) -> shader {
}
load_texture_from_file :: proc(file: string, alpha: bool) -> Texture2D {}

framebuffer_size_callback :: proc "c" (window: glfw.WindowHandle, width: i32, height: i32) {
	gl.Viewport(0, 0, width, height)
}

process_input :: proc "c" (window: glfw.WindowHandle) {
	if glfw.GetKey(window, glfw.KEY_ESCAPE) == glfw.PRESS {
		glfw.SetWindowShouldClose(window, true)
	}
}

main :: proc() {
	fmt.println("hey")
	glfw.Init()
	glfw.WindowHint(glfw.CONTEXT_VERSION_MAJOR, GL_MAJOR_V)
	glfw.WindowHint(glfw.CONTEXT_VERSION_MINOR, GL_MINOR_V)
	glfw.WindowHint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
	window := glfw.CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Breakout", nil, nil)
	if window == nil {
		fmt.println("[OPENGL ERROR] Failed to create GLFW window")
		glfw.Terminate()
		os.exit(-1)
	}

	glfw.MakeContextCurrent(window)
	gl.load_up_to(GL_MAJOR_V, GL_MINOR_V, glfw.gl_set_proc_address)

	gl.Viewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)

	glfw.SetFramebufferSizeCallback(window, framebuffer_size_callback)

	for !glfw.WindowShouldClose(window) {
		process_input(window)
		glfw.SwapBuffers(window)
		glfw.PollEvents()
	}

	glfw.Terminate()
}
