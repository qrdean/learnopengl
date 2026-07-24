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

vec2 :: [2]f32
vec3 :: [3]f32

/****** global managers *******/

resource_m: resource_manager = {}
renderer: sprite_renderer = {}

/****** player *******/

PLAYER_SIZE :: vec2{100., 20.}
PLAYER_VELOCITY :: 500.
player: game_object = {}
ball: ball_object = {}

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
}


update :: proc(self: ^game, dt: f32) {
	move_ball_object(&ball, dt, self.width)
	game_collisions(self)
  if (ball.position.y >= cast(f32)self.height) {
    reset_player(self) 
    reset_level(self) 
  }
}

render :: proc(self: game, dt: f32) {
	if (self.state == .ACTIVE) {
		draw_game_level(self.levels[self.level], &renderer)
		draw_game_object(player, &renderer)
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
  player.position = vec2{cast(f32)self.width / 2. - PLAYER_SIZE.x / 2., cast(f32)self.height - PLAYER_SIZE.y}
  reset_ball_object(&ball, player.position + vec2{PLAYER_SIZE.x / 2. - BALL_RADIUS, -(BALL_RADIUS * 2.)}, INITIAL_BALL_VELOCITY)
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
    strength: f32= 2.
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
