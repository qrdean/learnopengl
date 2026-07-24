package main

import "base:runtime"
import "core:fmt"
import "core:image"
import "core:image/png"
import "core:math/linalg"
import "core:math/linalg/glsl"
import "core:os"
import "core:strconv"
import "core:strings"

import gl "vendor:OpenGL"
import "vendor:glfw"

GL_MAJOR_V :: 3
GL_MINOR_V :: 3
WINDOW_WIDTH :: 800
WINDOW_HEIGHT :: 600


/****** global managers *******/

resource_m: resource_manager = {}
renderer: sprite_renderer = {}

/****** player *******/

PLAYER_SIZE :: vec2{100., 20.}
PLAYER_VELOCITY :: 500.
player: game_object = {}

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
  levels: [dynamic]game_level,
  level: u32,
}

create_game :: proc() -> game {
	return game{state = .ACTIVE, width = WINDOW_WIDTH, height = WINDOW_HEIGHT}
}

destroy_game :: proc(self: game) {}

game_init :: proc(self: ^game) {
	load_shader(&resource_m, "shaders/sprite_shader.vs", "shaders/sprite_shader.fs", "", "sprite")
	projection := glsl.mat4Ortho3d(0., cast(f32)self.width, cast(f32)self.height, 0., -1., 1.)
	set_integer_shader(use_shader(get_shader(resource_m, "sprite")), "image", 0)
	set_matrix_shader(get_shader(resource_m, "sprite"), "projection", projection)
	renderer = new_sprite_renderer(get_shader(resource_m, "sprite"))
	init_render_data(&renderer)
  // load textures
	load_texture(&resource_m, "textures/awesomeface.png", true, "face")
  load_texture(&resource_m, "textures/block.png", false, "block")
  load_texture(&resource_m, "textures/block_solid.png", false, "block_solid")
  load_texture(&resource_m, "textures/paddle.png", true, "paddle")
  // load levels
  one, two, three, four: game_level
  load_game_level(&one, "levels/one.lvl", self.width, self.height / 2)
  load_game_level(&two, "levels/two.lvl", self.width, self.height / 2)
  load_game_level(&three, "levels/three.lvl", self.width, self.height / 2)
  load_game_level(&four, "levels/four.lvl", self.width, self.height / 2)
  append(&self.levels, one)
  append(&self.levels, two)
  append(&self.levels, three)
  append(&self.levels, four)
  self.level = 0

  // player 
  player_pos := vec2{cast(f32)(self.width / 2.) - PLAYER_SIZE.x / 2., cast(f32)(self.height) - PLAYER_SIZE.y}
  player = new_game_object(player_pos, PLAYER_SIZE, get_texture(resource_m, "paddle"))
}


update :: proc(self: game, dt: f32) {
}
render :: proc(self: game, dt: f32) {
  if (self.state == .ACTIVE) {
    draw_game_level(self.levels[self.level], &renderer)
    draw_game_object(player, &renderer)
  }
	// texture := get_texture(resource_m, "face")
	// draw_sprite(&renderer, texture, {200., 200.}, {300., 400.}, 45., {0., 1., 0.})
}

game_level :: struct {
	bricks: [dynamic]game_object,
}

init_game_level :: proc(self: ^game_level, tile_data: [][]u32, level_width, level_height: u32) {
	height := len(tile_data)
	width := len(tile_data[0])
	unit_width := level_width / cast(u32)width
	unit_height := level_height / cast(u32)height
	for y in 0 ..< height {
		for x in 0 ..< width {
			if tile_data[y][x] == 1 { 	// solid
				pos := vec2 {
					cast(f32)(unit_width * cast(u32)x),
					cast(f32)(unit_height * cast(u32)y),
				}
				size := vec2{cast(f32)unit_width, cast(f32)unit_height}
				obj := new_game_object(
					pos,
					size,
					get_texture(resource_m, "block_solid"),
					vec3{0.8, 0.8, 0.7},
				)
				obj.is_solid = true
				append(&self.bricks, obj)
			} else if tile_data[y][x] > 1 {
				color := vec3{1., 1., 1.}
				if (tile_data[y][x] == 2) {
					color = vec3{0.2, 0.6, 1.}
				} else if (tile_data[y][x] == 3) {
					color = vec3{0.0, 0.7, 0.}
				} else if (tile_data[y][x] == 4) {
					color = vec3{0.8, 0.8, 0.4}
				} else if (tile_data[y][x] == 5) {
					color = vec3{1.0, 0.5, 0.}
				}

				pos := vec2 {
					cast(f32)(unit_width * cast(u32)x),
					cast(f32)(unit_height * cast(u32)y),
				}
				size := vec2{cast(f32)unit_width, cast(f32)unit_height}
				obj := new_game_object(pos, size, get_texture(resource_m, "block"), color)
				append(&self.bricks, obj)
			}
		}
	}
}

load_game_level :: proc(self: ^game_level, file: string, level_width, level_height: u32) {
	clear(&self.bricks)

	tile_code: u32
	level := game_level{}
	data, ok := read_entire_file(file, context.allocator)
	tile_data: [][]u32 = {}
	if !ok {
		fmt.println("err happened reading file")
		return
	}
	defer delete(data, context.allocator)

	file_contents := string(data)
	line_number := 0
  line_row: [dynamic][]u32
  defer delete(line_row)
	for line in strings.split_lines_iterator(&file_contents) {
		l := line
		row: [dynamic]u32
		defer delete(row)
		for elem in strings.split_iterator(&l, " ") {
			num, _ := strconv.parse_uint(elem)
			append(&row, cast(u32)num)
		}
    append(&line_row, row[:])
		line_number += 1
	}
  tile_data = line_row[:]

	if len(tile_data) > 0 {
		init_game_level(self, tile_data, level_width, level_height)
	}
}

draw_game_level :: proc(self: game_level, renderer: ^sprite_renderer) {
  for tile in self.bricks {
    if !tile.destroyed {
      draw_game_object(tile, renderer)
    }
  }
}

game_level_is_complete :: proc(self: game_level) -> bool {
  for tile in self.bricks {
    if !tile.is_solid && !tile.destroyed {
      return false
    }
  }
  return true
}

game_object :: struct {
	position:  vec2,
	size:      vec2,
	velocity:  vec2,
	color:     vec3,
	rotation:  f32,
	is_solid:  bool,
	destroyed: bool,
	sprite:    Texture2D,
}

new_game_object_default :: proc() -> game_object {
	return game_object {
		position = {0., 0.},
		size = {1., 1.},
		velocity = {0., 0.},
		color = {1., 1., 1.},
		rotation = 0.,
		sprite = {},
		is_solid = false,
		destroyed = false,
	}
}
new_game_object :: proc(
	pos, size: vec2,
	sprite: Texture2D,
	color: vec3 = {1., 1., 1.},
	velocity: vec2 = {0., 0.},
) -> game_object {
	return game_object {
		position = pos,
		size = size,
		velocity = velocity,
		color = color,
		rotation = 0.,
		sprite = sprite,
		is_solid = false,
		destroyed = false,
	}
}

draw_game_object :: proc(self: game_object, renderer: ^sprite_renderer) {
	draw_sprite(renderer, self.sprite, self.position, self.size, self.rotation, self.color)
}

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

vec2 :: [2]f32
vec3 :: [3]f32

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

framebuffer_size_callback :: proc "c" (window: glfw.WindowHandle, width: i32, height: i32) {
	gl.Viewport(0, 0, width, height)
}

process_input :: proc "c" (window: glfw.WindowHandle, game: ^game, dt: f32) {
	if glfw.GetKey(window, glfw.KEY_ESCAPE) == glfw.PRESS {
		glfw.SetWindowShouldClose(window, true)
	}

  if game.state == .ACTIVE {
    velocity := PLAYER_VELOCITY * dt
    if glfw.GetKey(window, glfw.KEY_A) == glfw.PRESS {
      if player.position.x >= 0. {
        player.position.x -= velocity
      }
    }
    if glfw.GetKey(window, glfw.KEY_D) == glfw.PRESS {
      if player.position.x <= cast(f32)game.width - player.size.x {
        player.position.x += velocity
      }
    }
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
	g := create_game()
	game_init(&g)

  delta_time :f32= 0.
  last_frame_time :f32= 0.
  accum_delta :f32= 0.

	for !glfw.WindowShouldClose(window) {

    current_frame_time := cast(f32)glfw.GetTime()
    delta_time = current_frame_time - last_frame_time
    last_frame_time = current_frame_time

		process_input(window, &g, delta_time)

		gl.ClearColor(0., 0., 0., 1.)
    gl.Clear(gl.COLOR_BUFFER_BIT)
    
		render(g, 0.)

		glfw.SwapBuffers(window)
		glfw.PollEvents()
	}

	glfw.Terminate()
}
