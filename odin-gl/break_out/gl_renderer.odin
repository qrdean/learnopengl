package main

import "core:fmt"
import "core:image"
import "core:image/png"
import "core:math/linalg/glsl"
import "core:os"
import "core:slice"

import gl "vendor:OpenGL"

_ :: png // registers the PNG decoder for image.load_from_file

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
	sVertex, sFragment: u32
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
			gl.GetProgramInfoLog(object, 1024, nil, raw_data(info_log[:]))
			fmt.printf("| ERROR::SHADER: Compile-time error: Type: %s \n", type)
		}
	} else {
		gl.GetProgramiv(object, gl.LINK_STATUS, &success)
		if success != 1 {
			gl.GetProgramInfoLog(object, 1024, nil, raw_data(info_log[:]))
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
		internal_format = gl.RGBA,
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
	vData, vErr := os.read_entire_file(vShaderFile, context.allocator)
	if vErr != nil {
		fmt.eprintf("[shader] cannot read %q: %v\n", vShaderFile, vErr)
		return {}
	}
	defer delete(vData)
	fData, fErr := os.read_entire_file(fShaderFile, context.allocator)
	if fErr != nil {
		fmt.eprintf("[shader] cannot read %q: %v\n", fShaderFile, fErr)
		return {}
	}
	defer delete(fData)
	program, ok := gl.load_shaders_source(string(vData), string(fData))
	if !ok {
		compile_msg, compile_type, link_msg, _ := gl.get_last_error_messages()
		if compile_msg != "" {
			fmt.eprintf("[shader] %v compile error:\n%s\n", compile_type, compile_msg)
		}
		if link_msg != "" {
			fmt.eprintf("[shader] link error:\n%s\n", link_msg)
		}
		if compile_msg == "" && link_msg == "" {
			fmt.eprintf("[shader] build failed with no logs: %q + %q\n", vShaderFile, fShaderFile)
		}
		return {}
	}
	return shader{id = program}
}

load_texture_from_file :: proc(file: string, alpha: bool) -> Texture2D {
	img, img_err := image.load_from_file(file)
	if img_err != nil {
		fmt.eprintf("[texture] cannot load %q: %v\n", file, img_err)
		return {}
	}
	defer free(img)

	t := new_texture_2d()
	if alpha {
		t.internal_format = gl.RGBA
		t.image_format = gl.RGBA
	}
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
	draw_sprite_rect(
		self,
		texture,
		Rect{0., 0., f32(texture.width), f32(texture.height)},
		position,
		size,
		rotate,
		color,
	)
}

draw_sprite_rect :: proc(
	self: ^sprite_renderer,
	texture: Texture2D,
	src: Rect,
	position: vec2,
	size: vec2 = {},
	rotate: f32 = 0.,
	color: vec3 = {1., 1., 1.},
) {
	dst := size
	if dst.x == 0. && dst.y == 0. {
		dst = {src.w, src.h}
	}

	use_shader(self.shader)
	model: glsl.mat4 = 1.
	model *= glsl.mat4Translate({position.x, position.y, 0.})
	model *= glsl.mat4Translate({0.5 * size.x, 0.5 * size.y, 0.})
	model *= glsl.mat4Rotate({0., 0., 1.}, glsl.radians(rotate))
	model *= glsl.mat4Translate({-.5 * dst.x, -0.5 * dst.y, 0.})
	model *= glsl.mat4Scale({dst.x, dst.y, 1.})

	set_matrix_shader(self.shader, "model", model)
	set_vector3f_shader(self.shader, "spriteColor", color.x, color.y, color.z)

	tw := f32(texture.width)
	th := f32(texture.height)
	set_vector4f_shader(
		self.shader,
		"uvRect",
		src.x / tw,
		src.y / th,
		(src.x + src.w) / tw,
		(src.y + src.h) / th,
	)

	gl.ActiveTexture(gl.TEXTURE0)
	bind_texture(texture)

	gl.BindVertexArray(self.quad_vao)
	gl.DrawArrays(gl.TRIANGLES, 0, 6)
	gl.BindVertexArray(0)
}

font_get_glyph :: proc(font: ^Font, r: rune) -> (glyph: Font_Baked_Glyph, ok: bool) {
	i, found := slice.binary_search_by(font.glyphs[:], r, proc(g: Font_Baked_Glyph, key: rune) -> slice.Ordering {
		if g.value < key do return .Less
		if g.value > key do return .Greater
		return .Equal
	})
	if !found {return {}, false}
	return font.glyphs[i], true
}

draw_text :: proc(
	self: ^sprite_renderer,
	font: ^Font,
	text: string,
	position: vec2,
	scale: f32 = 1.,
	color: vec3 = {1., 1., 1.},
) {
	line_height := (font.ascent - font.descent + font.line_gap) * scale
	pen := position

	for r in text {
		if r == '\n' {
			pen = {position.x, pen.y + line_height}
			continue
		}

		g, found := font_get_glyph(font, r)
		if !found {continue}

		if g.rect.w > 0. && g.rect.h > 0. {
			draw_sprite_rect(
				self,
				font.texture,
				g.rect,
				{pen.x + g.offset.x * scale, pen.y + g.offset.y * scale},
				{g.rect.w * scale, g.rect.h * scale},
				0.,
				color,
			)
		}
		pen.x += g.advance * scale
	}
}
