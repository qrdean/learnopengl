package main

import "base:runtime"
import "core:fmt"
import "core:image"
import "core:image/png"
import "core:math/linalg"
import "core:math/linalg/glsl"
import "core:math/rand"
import "core:os"
import "core:slice"
import "core:strconv"
import "core:strings"
import "core:unicode/utf8"
import stbrp "vendor:stb/rect_pack"
import stbtt "vendor:stb/truetype"

import gl "vendor:OpenGL"
import "vendor:glfw"

GL_MAJOR_V :: 3
GL_MINOR_V :: 3
WINDOW_WIDTH :: 800
WINDOW_HEIGHT :: 600

vec2 :: [2]f32
vec3 :: [3]f32
vec4 :: [4]f32

Rect :: struct {
	x, y: f32,
	w, h: f32,
}

/****** global managers *******/

resource_m: resource_manager = {}
renderer: sprite_renderer = {}

/****** player *******/

PLAYER_SIZE :: vec2{100., 20.}
PLAYER_VELOCITY :: 500.
player: game_object = {}
ball: ball_object = {}
MAX_PARTICLES :: 500
particle_gen: particle_generator = {}

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
	level:  u32,
}

create_game :: proc() -> game {
	return game{state = .ACTIVE, width = WINDOW_WIDTH, height = WINDOW_HEIGHT}
}

destroy_game :: proc(self: game) {}

game_init :: proc(self: ^game) {
	// sprite shader
	load_shader(&resource_m, "shaders/sprite_shader.vs", "shaders/sprite_shader.fs", "", "sprite")
	projection := glsl.mat4Ortho3d(0., cast(f32)self.width, cast(f32)self.height, 0., -1., 1.)
	set_integer_shader(use_shader(get_shader(resource_m, "sprite")), "image", 0)
	set_matrix_shader(get_shader(resource_m, "sprite"), "projection", projection)

	// particle shader
	load_shader(
		&resource_m,
		"shaders/particle_shader.vs",
		"shaders/particle_shader.fs",
		"",
		"particle",
	)
	set_matrix_shader(get_shader(resource_m, "particle"), "projection", projection, true)

	renderer = new_sprite_renderer(get_shader(resource_m, "sprite"))
	init_render_data(&renderer)
	// load textures
	load_texture(&resource_m, "textures/awesomeface.png", true, "face")
	load_texture(&resource_m, "textures/block.png", false, "block")
	load_texture(&resource_m, "textures/block_solid.png", false, "block_solid")
	load_texture(&resource_m, "textures/paddle.png", true, "paddle")
	load_texture(&resource_m, "textures/particle.png", true, "particle")
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
	player_pos := vec2 {
		cast(f32)(self.width / 2.) - PLAYER_SIZE.x / 2.,
		cast(f32)(self.height) - PLAYER_SIZE.y,
	}
	player = new_game_object(player_pos, PLAYER_SIZE, get_texture(resource_m, "paddle"))

	// ball
	ball_pos := player_pos + vec2{PLAYER_SIZE.x / 2. - BALL_RADIUS, -BALL_RADIUS * 2.}
	ball = new_ball_object(
		ball_pos,
		BALL_RADIUS,
		INITIAL_BALL_VELOCITY,
		get_texture(resource_m, "face"),
	)

	particle_gen = new_particle_gen(
		get_shader(resource_m, "particle"),
		get_texture(resource_m, "particle"),
		MAX_PARTICLES,
	)

	init_particle_gen(&particle_gen)

	font_codepoints := utf8.string_to_runes(
		"abcdefghiklmnopqrstuvwxyzåäöABCDEFGHIKLMNOPQRSTUVWXYZÅÄÖ!()1234567890., :",
		context.temp_allocator,
	)

	load_static_font_from_bytes(#load("fonts/roboto.ttf"), 48, font_codepoints)
}


update :: proc(self: ^game, dt: f32) {
	move_ball_object(&ball, dt, self.width)
	game_collisions(self)
	update_particle_gen(&particle_gen, dt, ball, 2, vec2{ball.radius / 2., ball.radius / 2.})
	if (ball.position.y >= cast(f32)self.height) {
		reset_player(self)
		reset_level(self)
	}
}

render :: proc(self: game, dt: f32) {
	if (self.state == .ACTIVE) {
		draw_game_level(self.levels[self.level], &renderer)
		draw_game_object(player, &renderer)
		draw_particle_gen(particle_gen)
		draw_game_object(ball, &renderer)
	}
	// texture := get_texture(resource_m, "face")
	// draw_sprite(&renderer, texture, {200., 200.}, {300., 400.}, 45., {0., 1., 0.})
}

reset_level :: proc(self: ^game) {
	if (self.level == 0) {
		load_game_level(&self.levels[0], "levels/one.lvl", self.width, self.height / 2)
	}
	if (self.level == 1) {
		load_game_level(&self.levels[1], "levels/two.lvl", self.width, self.height / 2)
	}
	if (self.level == 2) {
		load_game_level(&self.levels[1], "levels/three.lvl", self.width, self.height / 2)
	}
	if (self.level == 3) {
		load_game_level(&self.levels[1], "levels/four.lvl", self.width, self.height / 2)
	}
}

reset_player :: proc(self: ^game) {
	player.size = PLAYER_SIZE
	player.position = vec2 {
		cast(f32)self.width / 2. - PLAYER_SIZE.x / 2.,
		cast(f32)self.height - PLAYER_SIZE.y,
	}
	reset_ball_object(
		&ball,
		player.position + vec2{PLAYER_SIZE.x / 2. - BALL_RADIUS, -(BALL_RADIUS * 2.)},
		INITIAL_BALL_VELOCITY,
	)
}

game_collisions :: proc(self: ^game) {
	for &box in self.levels[self.level].bricks {
		if !box.destroyed {
			collision, direction, diff_vector := check_ball_collision(ball, box)
			if collision {
				if !box.is_solid {
					box.destroyed = true
				}

				switch direction {
				case .LEFT:
					ball.velocity.x = -ball.velocity.x
					penetration := ball.radius - linalg.abs(diff_vector.x)
					ball.position.x += penetration
				case .RIGHT:
					ball.velocity.x = -ball.velocity.x
					penetration := ball.radius - linalg.abs(diff_vector.x)
					ball.position.x -= penetration
				case .UP:
					ball.velocity.y = -ball.velocity.y
					penetration := ball.radius - linalg.abs(diff_vector.y)
					ball.position.y -= penetration
				case .DOWN:
					ball.velocity.y = -ball.velocity.y
					penetration := ball.radius - linalg.abs(diff_vector.y)
					ball.position.y += penetration
				}
			}
		}
	}

	collision, direction, diff_vector := check_ball_collision(ball, player)
	if (!ball.stuck && collision) {
		center_board := player.position.x + player.size.x / 2.
		distance := (ball.position.x + ball.radius) - center_board
		percentage := distance / player.size.x / 2.
		strength: f32 = 2.
		old_velocity := ball.velocity
		ball.velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength
		ball.velocity.y = -ball.velocity.y
		ball.velocity = linalg.normalize(ball.velocity) * linalg.length(old_velocity)
	}
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
		for elem in strings.split_iterator(&l, " ") {
			num, _ := strconv.parse_uint(elem)
			append(&row, cast(u32)num)
		}
		row_slice := make([]u32, len(row))
		copy(row_slice, row[:])
		append(&line_row, row_slice)
		delete(row)
		line_number += 1
	}
	tile_data = line_row[:]

	if len(tile_data) > 0 {
		init_game_level(self, tile_data, level_width, level_height)
	}

	for rs in line_row {
		delete(rs)
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


check_collision :: proc(one: game_object, two: game_object) -> bool {
	collision_x :=
		one.position.x + one.size.x >= two.position.x &&
		two.position.x + two.size.x >= one.position.x
	collision_y :=
		one.position.y + one.size.y >= two.position.y &&
		two.position.y + two.size.y >= one.position.y
	return collision_x && collision_y
}

check_ball_collision :: proc(one: ball_object, two: game_object) -> (bool, Direction, vec2) {
	center := one.position + one.radius
	aabb_half_extents := vec2{two.size.x / 2., two.size.y / 2.}
	aabb_center := vec2{two.position.x + aabb_half_extents.x, two.position.y + aabb_half_extents.y}

	difference := center - aabb_center
	clamped := linalg.clamp(difference, -aabb_half_extents, aabb_half_extents)
	closest := aabb_center + clamped
	difference = closest - center
	if linalg.length(difference) <= one.radius {
		return true, vector_direction(difference), difference
	}
	return false, .UP, vec2{0., 0.}
}

INITIAL_BALL_VELOCITY :: vec2{100., -350.}
BALL_RADIUS :: 12.5

Direction :: enum {
	UP,
	RIGHT,
	DOWN,
	LEFT,
}

int_to_direction :: proc(i: int) -> Direction {
	if i == 0 {
		return .UP
	}
	if i == 1 {
		return .RIGHT
	}
	if i == 2 {
		return .DOWN
	}
	if i == 3 {
		return .LEFT
	}
	return .UP
}

vector_direction :: proc(target: vec2) -> Direction {
	compass_size :: 4
	compass := [compass_size]vec2{{0., 1.}, {1., 0.}, {0., -1.}, {-1., 0.}}

	max: f32 = 0.
	best_match := Direction.UP
	for i in 0 ..< 4 {
		dot_product := linalg.dot(linalg.normalize(target), compass[i])
		if dot_product > max {
			max = dot_product
			best_match = int_to_direction(i)
		}
	}
	return best_match
}


ball_object :: struct {
	using game_object: game_object,
	radius:            f32,
	stuck:             bool,
}

new_ball_object :: proc(pos: vec2, radius: f32, velocity: vec2, sprite: Texture2D) -> ball_object {
	obj := new_game_object(pos, {radius * 2., radius * 2.}, sprite, {1., 1., 1.}, velocity)
	return ball_object{game_object = obj, radius = radius, stuck = true}
}

move_ball_object :: proc(self: ^ball_object, dt: f32, window_width: u32) -> vec2 {
	if !self.stuck {
		self.position += self.velocity * dt
		if self.position.x <= 0. {
			self.velocity.x = -self.velocity.x
			self.position.x = 0.
		} else if self.position.x + self.size.x >= cast(f32)window_width {
			self.velocity.x = -self.velocity.x
			self.position.x = cast(f32)window_width - self.size.x
		}

		if self.position.y <= 0. {
			self.velocity.y = -self.velocity.y
			self.position.y = 0.
		}
	}
	return self.position
}

reset_ball_object :: proc(self: ^ball_object, pos, velocity: vec2) {
	self.position = pos
	self.velocity = velocity
	self.stuck = true
}

particle :: struct {
	position: vec2,
	velocity: vec2,
	color:    vec4,
	life:     f32,
}

new_particle_default :: proc() -> particle {
	return particle{position = {0., 0.}, velocity = {0., 0.}, color = {1., 1., 1., 1.}, life = 0.}
}

new_particle :: proc(pos, vel: vec2, color: vec4, life: f32) -> particle {
	return particle{position = pos, velocity = vel, color = color, life = life}
}

particle_generator :: struct {
	particles: [dynamic]particle,
	amount:    u32,
	shader:    shader,
	texture:   Texture2D,
	vao:       u32,
}

new_particle_gen :: proc(shader: shader, texture: Texture2D, amount: u32) -> particle_generator {
	return particle_generator{shader = shader, texture = texture, amount = amount}
}

init_particle_gen :: proc(self: ^particle_generator) {
	vbo: u32
	particle_quad := VERTICES
	gl.GenVertexArrays(1, &self.vao)
	gl.GenBuffers(1, &vbo)
	gl.BindVertexArray(self.vao)
	gl.BindBuffer(gl.ARRAY_BUFFER, vbo)
	gl.BufferData(
		gl.ARRAY_BUFFER,
		size_of(particle_quad),
		raw_data(&particle_quad),
		gl.STATIC_DRAW,
	)
	gl.EnableVertexAttribArray(0)
	gl.VertexAttribPointer(0, 4, gl.FLOAT, gl.FALSE, 4 * size_of(f32), 0)
	gl.BindVertexArray(0)
	for i in 0 ..< self.amount {
		append(&self.particles, new_particle_default())
	}
}

update_particle_gen :: proc(
	self: ^particle_generator,
	dt: f32,
	object: game_object,
	new_particles: u32,
	offset: vec2 = {0., 0.},
) {
	nr_new_particles := 2
	for i in 0 ..< nr_new_particles {
		unused_particle := first_unused_particle(self)
		respawn_particles(&self.particles[unused_particle], object, offset)
	}

	for i in 0 ..< MAX_PARTICLES {
		particle := &self.particles[i]
		particle.life -= dt
		if particle.life > 0. {
			particle.position -= particle.velocity * dt
			particle.color.a -= dt * 2.5
		}
	}
}

draw_particle_gen :: proc(self: particle_generator) {
	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE)
	use_shader(self.shader)
	for p in self.particles {
		if p.life > 0. {
			set_vector2f_shader(self.shader, "offset", p.position.x, p.position.y)
			set_vector4f_shader(self.shader, "color", p.color.x, p.color.y, p.color.z, p.color.a)
			// gl.ActiveTexture(gl.TEXTURE0)
			bind_texture(self.texture)
			gl.BindVertexArray(self.vao)
			gl.DrawArrays(gl.TRIANGLES, 0, 6)
			gl.BindVertexArray(0)
		}
	}
	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
}

last_used_particle := 0

first_unused_particle :: proc(self: ^particle_generator) -> u32 {
	// returns almost immediately unless we reach max
	for i in last_used_particle ..< MAX_PARTICLES {
		if self.particles[i].life <= 0. {
			last_used_particle = i
			return cast(u32)i
		}
	}
	for i in 0 ..< last_used_particle {
		if self.particles[i].life <= 0. {
			last_used_particle = i
			return cast(u32)i
		}
	}

	// override first particle if all others are alive
	last_used_particle = 0
	return 0
}

respawn_particles :: proc(p: ^particle, object: game_object, offset: vec2 = {0., 0.}) {
	random := (rand.float32() - 0.5) * 10.
	r_color := 0.5 + ((rand.float32() * 100.) / 100.)
	p.position = object.position + random + offset
	p.color = vec4{r_color, r_color, r_color, 1.}
	p.life = 1.
	p.velocity = object.velocity * 0.1
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
				if ball.stuck {
					ball.position.x -= velocity
				}
			}
		}
		if glfw.GetKey(window, glfw.KEY_D) == glfw.PRESS {
			if player.position.x <= cast(f32)game.width - player.size.x {
				player.position.x += velocity
				if ball.stuck {
					ball.position.x += velocity
				}
			}
		}
		if glfw.GetKey(window, glfw.KEY_SPACE) == glfw.PRESS {
			ball.stuck = false
		}
	}
}

Texture_Filter :: enum {
	Point,
	Linear,
}

Font_Options :: struct {
	premultiply_alpha: bool,
	filter:            Texture_Filter,
}

Font_Type :: enum {
	Static,
	Dynamic,
}

// Font_Data :: struct {}

Font_Baked_Glyph_Range :: struct {
	start_idx: int,
	start:     rune,
	end:       rune,
}

Font_Baked_Glyph :: struct {
	value:   rune,
	index:   int,
	rect:    Rect,
	offset:  vec2,
	advance: f32,
}

Image :: struct {
	pixels: []vec4,
	width:  int,
	height: int,
}

load_static_font_from_bytes :: proc(
	data: []byte,
	font_size: f32,
	codepoints: []rune = {},
	options: Font_Options = {},
) {
	codepoints := codepoints
	font_info: stbtt.fontinfo
	font_offset := stbtt.GetFontOffsetForIndex(raw_data(data), 0)
	init_ok := stbtt.InitFont(&font_info, raw_data(data), font_offset)

	if !init_ok {
		fmt.println("failed loading ttf/ttc font")
		return
	}

	scale_factor := stbtt.ScaleForPixelHeight(&font_info, font_size)

	ascent, descent, line_gap: i32
	stbtt.GetFontVMetrics(&font_info, &ascent, &descent, &line_gap)

	default_codepoints: [95]rune

	if len(codepoints) == 0 {
		for &d, idx in default_codepoints {
			d = rune(idx + 32)
		}

		codepoints = default_codepoints[:]
	}

	glyph_ranges := make([dynamic]Font_Baked_Glyph_Range, context.allocator)
	glyphs := make([dynamic]Font_Baked_Glyph, context.allocator)

	for c in codepoints {
		idx := stbtt.FindGlyphIndex(&font_info, c)

		if idx > 0 {
			advance: i32
			stbtt.GetGlyphHMetrics(&font_info, idx, &advance, nil)

			append(
				&glyphs,
				Font_Baked_Glyph {
					value = c,
					index = int(idx),
					advance = f32(advance) * scale_factor,
				},
			)
		}
	}

	slice.sort_by(glyphs[:], proc(i, j: Font_Baked_Glyph) -> bool {
		return i.value < j.value
	})

	cur_glyph_range: Font_Baked_Glyph_Range

	for g, g_idx in glyphs {
		if g_idx == 0 {
			cur_glyph_range = {
				start     = g.value,
				start_idx = g_idx,
			}
		} else if g.value != cur_glyph_range.end {
			append(&glyph_ranges, cur_glyph_range)
			cur_glyph_range = {
				start     = g.value,
				start_idx = g_idx,
			}
		}

		cur_glyph_range.end = g.value + 1
	}

	Glyph_Image_Data :: struct {
		pixels: [^]u8,
		width:  i32,
		height: i32,
	}

	glyphs_image_data := make([dynamic]Glyph_Image_Data, context.allocator)
	glyphs_font_rects := make([dynamic]stbrp.Rect, context.allocator)

	for &g, g_idx in glyphs {
		x_offset: i32
		y_offset: i32

		width: i32
		height: i32

		pixels := stbtt.GetGlyphBitmap(
			&font_info,
			scale_factor,
			scale_factor,
			i32(g.index),
			&width,
			&height,
			&x_offset,
			&y_offset,
		)

		glyphs_image_data[g_idx] = {
			pixels = pixels,
			width  = width,
			height = height,
		}

		g.offset = {f32(x_offset), f32(y_offset) + f32(ascent) * scale_factor}

		glyphs_font_rects[g_idx] = {
			w = stbrp.Coord(width) + 1,
			h = stbrp.Coord(height) + 1,
		}

		atlas_size := 128
		MAX_ATLAS_SIZE :: 4096
		atlas_packed := false

		for atlas_size <= MAX_ATLAS_SIZE {
			// start packing rect for this iteration
			rp_ctx: stbrp.Context
			rp_nodes := make([]stbrp.Node, i32(atlas_size), context.allocator)

			stbrp.init_target(
				&rp_ctx,
				i32(atlas_size),
				i32(atlas_size),
				raw_data(rp_nodes),
				i32(len(rp_nodes)),
			)

			rect_pack_res := stbrp.pack_rects(
				&rp_ctx,
				raw_data(glyphs_font_rects),
				i32(len(glyphs_font_rects)),
			)

			if rect_pack_res == 1 {
				atlas_packed = true
				break
			}

			atlas_size *= 2
		}

		if !atlas_packed {
			fmt.println("failed to pack font atlas")
			return
		}


		atlas := make([]vec4, atlas_size * atlas_size, context.allocator)

		// TODO: Implement this option
		if options.premultiply_alpha {} else {
			for pr, pr_idx in glyphs_font_rects {
				g := &glyphs[pr_idx]
				g.rect = {f32(pr.x), f32(pr.y), f32(pr.w) - 1, f32(pr.h) - 1}

				g_img := glyphs_image_data[pr_idx]
				for sx in 0 ..< g_img.width {
					for sy in 0 ..< g_img.height {
						dx := int(pr.x) + int(sx)
						dy := int(pr.y) + int(sy)

						assert(dx >= 0 && dx < atlas_size)
						assert(dy >= 0 && dy < atlas_size)

						alpha := g_img.pixels[sy * g_img.width + sx] / 255
						atlas[dy * atlas_size + dx] = {1.0, 1.0, 1.0, f32(alpha)}
					}
				}
			}
		}
		for g_img_data in glyphs_image_data {
			if g_img_data.pixels != nil {
				stbtt.FreeBitmap(g_img_data.pixels, nil)
			}
		}

		img := Image {
			pixels = atlas,
      width = atlas_size,
      height = atlas_size,
		}

    tex := load_texture_from_image(img)
    // set_texture_filter(tex, options.filter)
	}


	fmt.println("loaded the file")
}

load_texture_from_image :: proc(image: Image) -> Texture2D {
  if image.width == 0 || image.height == 0 {
    fmt.println("invalid image height or width is 0")
    return {}
  }

  if len(image.pixels) != (image.width * image.height) {
    fmt.printf("invalid image pixels array is not of size %d x %d\n", image.width, image.height)
    return {}
  }

  return {
    handle
  }
}


main :: proc() {
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

	delta_time: f32 = 0.
	last_frame_time: f32 = 0.
	accum_delta: f32 = 0.

	for !glfw.WindowShouldClose(window) {

		current_frame_time := cast(f32)glfw.GetTime()
		delta_time = current_frame_time - last_frame_time
		last_frame_time = current_frame_time

		process_input(window, &g, delta_time)
		update(&g, delta_time)

		gl.ClearColor(0., 0., 0., 1.)
		gl.Clear(gl.COLOR_BUFFER_BIT)

		render(g, delta_time)

		glfw.SwapBuffers(window)
		glfw.PollEvents()
	}

	glfw.Terminate()
}
