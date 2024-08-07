bool almost_equals(float a, float b, float epsilon){
	return fabs(a-b) <= epsilon;
}

bool animate_f32_to_target(float* value, float target, float delta_t, float rate){
	*value += (target - *value) * (1.0 - pow(2.0f, -rate * delta_t));
	if (almost_equals(*value, target, 0.001f))
	{
		*value = target;
		return true;
	}
	return false;
}

bool animate_v2_to_target(Vector2* value, Vector2 target, float delta_t, float rate){
	animate_f32_to_target(&(value->x), target.x, delta_t, rate);
	animate_f32_to_target(&(value->y), target.y, delta_t, rate);
}

typedef struct Sprite {
	Gfx_Image* image;
	Vector2 size;
} Sprite;

typedef enum SpriteID {
	SPRITE_nil,
	SPRITE_player,
	SPRITE_tree0,
	SPRITE_rock0,
	SPRITE_MAX,
} SpriteID;
// dakota: maybe we make this an X macro?? https://chatgpt.com/share/260222eb-2738-4d1e-8b1d-4973a097814d
Sprite sprites[SPRITE_MAX];

Sprite* get_sprite(SpriteID id){
	if (id >= 0 && id < SPRITE_MAX) {
		return &sprites[id];
	}
	return &sprites[0];

}

typedef enum EntityArchetype {
	arch_nil = 0,
	arch_rock = 1,
	arch_tree = 2,
	arch_player = 3,
}EntityArchetype;

typedef struct Entity {
	bool is_valid;
	EntityArchetype arch;
	Vector2 pos;

	bool render_sprite;
	SpriteID sprite_id;
}Entity;
#define MAX_ENTITY_COUNT 1024

typedef struct World {
	Entity entities[MAX_ENTITY_COUNT];
}World;

World* world = 0;

Entity* entity_create(){
	Entity* entity_found = 0;
	for (int i=0; i < MAX_ENTITY_COUNT; i++){
		Entity* existing_entity = &world->entities[i];
		if (!existing_entity->is_valid) {
			entity_found = existing_entity;
			break;
		}
	}
	assert(entity_found, "No more free entities!");
	entity_found->is_valid=true;
	return entity_found;
}

void entity_destory(Entity* entity) {
	memset(entity, 0, sizeof(Entity));
}

void setup_rock(Entity* en){
	en->arch = arch_rock;
	en->sprite_id = SPRITE_rock0;
}
void setup_tree(Entity* en){
	en->arch = arch_tree;
	en->sprite_id = SPRITE_tree0;
}
void setup_player(Entity* en){
	en->arch = arch_player;
	en->sprite_id = SPRITE_player;
}


int entry(int argc, char **argv) {
	
	window.title = fixed_string("Slime Fort");
	window.width = 1280;
	window.height = 720;
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x2f3640ff);

	world = alloc(get_heap_allocator(), sizeof(World));

	sprites[SPRITE_player] = (Sprite){.image=load_image_from_disk(fixed_string("assets\\player.png"), get_heap_allocator()), .size = v2(6.0,8.0)};
	sprites[SPRITE_tree0] = (Sprite){.image=load_image_from_disk(fixed_string("assets\\tree0.png"), get_heap_allocator()), .size =  v2(10.0,14.0)};
	sprites[SPRITE_rock0] = (Sprite){.image=load_image_from_disk(fixed_string("assets\\rock0.png"), get_heap_allocator()), .size = v2(8.0,4.0)};


	Entity* player_en = entity_create();
	setup_player(player_en);

	for (int i = 0; i < 10; i++){
		Entity* en = entity_create();
		setup_rock(en);
		en->pos = v2(get_random_float32_in_range(-100,100),get_random_float32_in_range(-100,100));
	}
	for (int i = 0; i < 10; i++){
		Entity* en = entity_create();
		setup_tree(en);
		en->pos = v2(get_random_float32_in_range(-100,100),get_random_float32_in_range(-100,100));
	}

	float64 seconds_counter = 0.0;
	s32 frame_count = 0;

	float zoom = 5.3;
	Vector2 camera_pos = v2(0,0);

	float64 last_time = os_get_current_time_in_seconds();
	while (!window.should_close) {
		reset_temporary_storage();

		float64 now = os_get_current_time_in_seconds();
		float64 delta_t = now - last_time;
		last_time = now;

		//Overide the cordainate system to be the same size as the window
		draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5,-1,10);

		// :camera
		{
			Vector2 target_pos = player_en->pos;
			animate_v2_to_target(&camera_pos, target_pos, delta_t, 15.0f);

			draw_frame.view = m4_make_scale(v3(1.0,1.0,1.0));
			draw_frame.view = m4_mul(draw_frame.view, m4_make_translation(v3(camera_pos.x, camera_pos.y,0)));
			draw_frame.view = m4_mul(draw_frame.view, m4_make_scale(v3(1.0/zoom,1.0/zoom,1.0)));
		}


		os_update();

		for (int i = 0; i<MAX_ENTITY_COUNT; i++){
			Entity* en = &world->entities[i];
			if(en->is_valid){
				switch(en->arch){
					default:
					{
						Sprite* sprite = get_sprite(en->sprite_id);
						Matrix4 xform = m4_scalar(1.0);
						xform = m4_translate(xform, v3(en->pos.x,en->pos.y,0));
						xform = m4_translate(xform, v3(sprite->size.x * -.5,0,0));
						draw_image_xform(sprite->image,xform, sprite->size, COLOR_WHITE);
					}
				}
			}
		}

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
		player_en->pos = v2_add(player_en->pos, v2_mulf(input_axis, 50 * delta_t));



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