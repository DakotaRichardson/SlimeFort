
int entry(int argc, char **argv) {
	
window.title = fixed_string("Slime Fort");
window.width = 1280;
window.height = 720;
window.x = 200;
window.y = 200;
window.clear_color = hex_to_rgba(0x2f3640ff);

Gfx_Image* player = load_image_from_disk(fixed_string("assets\\player.png"), get_heap_allocator());
assert(player, "Player Image Fuckie Wuckied");

Vector2 player_pos = v2(0,0);
float64 seconds_counter = 0.0;
s32 frame_count = 0;

float64 last_time = os_get_current_time_in_seconds();
while (!window.should_close) {
	reset_temporary_storage();

	//Overide the cordainate system to be the same size as the window
	draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5,-1,10);
	// Zoom in camera
	float zoom = 5.3;
	draw_frame.view = m4_make_scale(v3(1.0/zoom,1.0/zoom,1));

	float64 now = os_get_current_time_in_seconds();
	float64 delta_t = now - last_time;
	last_time = now;

	os_update();

	if (is_key_just_pressed(KEY_ESCAPE)){
		window.should_close = true;
	}

	Vector2 input_axis = v2(0,0);
	if (is_key_down('A')){
		input_axis.x -= 1.0;
	}
	if (is_key_down('D')){
		input_axis.x += 1.0;
	}
	if (is_key_down('S')){
		input_axis.y -= 1.0;
	}
	if (is_key_down('W')){
		input_axis.y += 1.0;
	}
	input_axis = v2_normalize(input_axis);



	//player_pos = player_pos + (input_axis * 10)
	player_pos = v2_add(player_pos, v2_mulf(input_axis, 50 * delta_t));

	Vector2 size = v2(6.0,8.0);
	Matrix4 xform = m4_scalar(1.0);
	xform = m4_translate(xform, v3(player_pos.x,player_pos.y,0));
	xform = m4_translate(xform, v3(size.x * -.5,0,0));
	draw_image_xform(player,xform, v2(12.0f, 9.0f), COLOR_RED);

	gfx_update();
	seconds_counter += delta_t;
	frame_count += 1;
	if (seconds_counter > 1.0) {
		log("fps: %i", frame_count);
		seconds_counter = 0;
		frame_count = 0;
	}
}
return 0;
}