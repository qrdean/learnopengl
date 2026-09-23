package main

import "base:runtime"
import "core:fmt"
import "core:image"
import "core:image/png"
import "core:math/linalg/glsl"

import gl "vendor:OpenGL"

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

set_matrix_shader :: proc(self: shader, name: cstring, matx: glsl.mat4, use: bool = false) {
	if use {
		use_shader(self)
	}
	m := matx
	gl.UniformMatrix4fv(gl.GetUniformLocation(self.id, name), 1, false, raw_data(&m))
}

check_compile_errors :: proc(object: u32, type: string) {
	success: i32
	info_log: [1024]u8
	if type != "PROGRAM" {
		gl.GetShaderiv(object, gl.COMPILE_STATUS, &success)
		if success != 1 {
			gl.GetShaderInfoLog(object, 1024, nil, raw_data(info_log[:]))
			fmt.printf("| ERROR::SHADER: Compile-time error: Type: %s \n", type)
		}
	} else {
		gl.GetProgramiv(object, gl.LINK_STATUS, &success)
		if success != 1 {
			gl.GetShaderInfoLog(object, 1024, nil, raw_data(info_log[:]))
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

bind_texture :: proc(self: Texture2D) {
	gl.BindTexture(gl.TEXTURE_2D, self.Id)
}

resource_manager :: struct {
	shaders:  map[string]shader,
	textures: map[string]Texture2D,
}

new_resource_manager :: proc() -> resource_manager {
	return resource_manager{}
}

load_shader :: proc(
	rm: ^resource_manager,
	vShaderFile, fShaderFile, gShaderFile, name: string,
) -> shader {
	rm.shaders[name] = load_shader_from_file(vShaderFile, fShaderFile, gShaderFile)
	return rm.shaders[name]
}

get_shader :: proc(rm: resource_manager, name: string) -> shader {
	return rm.shaders[name]
}

load_texture :: proc(rm: ^resource_manager, file: string, alpha: bool, name: string) -> Texture2D {
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
	vData, ok := read_entire_file(vShaderFile, context.allocator)
	if !ok {
		fmt.printf("ERROR reading vertex file\n")
		return {}
	}
	fData, ok_2 := read_entire_file(fShaderFile, context.allocator)
	if !ok_2 {
		fmt.printf("ERROR reading fragment file\n")
		return {}
	}
	s: shader
	v_shader_copy := cstring(raw_data(vData))
	f_shader_copy := cstring(raw_data(fData))
	compile_shader(&s, &v_shader_copy, &f_shader_copy)
	return s
}

load_texture_from_file :: proc(file: string, alpha: bool) -> Texture2D {
	t := new_texture_2d()
	img, img_err := image.load_from_file(file)
	if img_err != nil {
		fmt.printf("img_err: %v\n", img_err)
		return {}
	}
	defer free(img)

	if alpha {
		t.internal_format = gl.RGBA
		t.image_format = gl.RGBA
	}
	// id: u32
	generate_texture_2d(&t, cast(i32)img.width, cast(i32)img.height, raw_data(img.pixels.buf[:]))
	return t
}

sprite_renderer :: struct {
	shader:   shader,
	quad_vao: u32,
}

new_sprite_renderer :: proc(shader: shader) -> sprite_renderer {
	return sprite_renderer{shader = shader}
}

init_render_data :: proc(self: ^sprite_renderer) {
	VBO: u32 = 0

	// vertices := [?]f32  {
	//   0., 1., 0., 1.,
	//   1., 0., 1., 0.,
	//   0., 0., 0., 0.,
	//
	//   0., 1., 0., 1.,
	//   1., 1., 1., 1.,
	//   1., 0., 1., 0.
	// }
	vertices := VERTICES

	gl.GenVertexArrays(1, &self.quad_vao)
	gl.GenBuffers(1, &VBO)

	gl.BindBuffer(gl.ARRAY_BUFFER, VBO)
	gl.BufferData(gl.ARRAY_BUFFER, size_of(vertices), raw_data(&vertices), gl.STATIC_DRAW)

	gl.BindVertexArray(self.quad_vao)
	gl.EnableVertexAttribArray(0)
	gl.VertexAttribPointer(0, 4, gl.FLOAT, gl.FALSE, 4 * size_of(f32), 0)
	gl.BindBuffer(gl.ARRAY_BUFFER, 0)
	gl.BindVertexArray(0)
}

draw_sprite :: proc(
	self: ^sprite_renderer,
	texture: Texture2D,
	position: vec2,
	size: vec2 = {10., 10.},
	rotate: f32 = 0.,
	color: vec3 = {1., 1., 1.},
) {
	use_shader(self.shader)
	model: glsl.mat4 = 1.
	model *= glsl.mat4Translate({position.x, position.y, 0.})

	model *= glsl.mat4Translate({0.5 * size.x, 0.5 * size.y, 0.})
	model *= glsl.mat4Rotate({0., 0., 1.}, glsl.radians(rotate))
	model *= glsl.mat4Translate({-.5 * size.x, -0.5 * size.y, 0.})
	model *= glsl.mat4Scale({size.x, size.y, 1.})

	set_matrix_shader(self.shader, "model", model)
	set_vector3f_shader(self.shader, "spriteColor", color.x, color.y, color.z)

	gl.ActiveTexture(gl.TEXTURE0)
	bind_texture(texture)

	gl.BindVertexArray(self.quad_vao)
	gl.DrawArrays(gl.TRIANGLES, 0, 6)
	gl.BindVertexArray(0)
}
