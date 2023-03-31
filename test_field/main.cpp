#include "spock.hpp"
#include "utility/image_loader.hpp"

enum current_state_e: uint32_t {
    STATE_MENU,
    STATE_LOAD,
    STATE_PLAY,
    STATE_EXIT_PLAY,
    STATE_EXIT
};

MAIN {
    spk_trace();

    spk::log.log("running..");
    int code = 0;

    SPK_DEBUG_ENABLE(spk::DEBUG_FLAGS_ENABLE_ENGINE_LIFETIME);

    spk::init();

    spk::print_deps_versions();

    spk::settings_t&  settings  = spk::settings();
    spk::resources_t& resources = spk::resources();
    spk::scene_t&     scene     = spk::scene();
    spk::canvas_t&    canvas    = spk::canvas();
    b2World*          world = scene.physics_world;
    uint32_t          smells_blood_id, coin_sound_id, font_id;
    uint32_t          sprite_array_id, high_def_array;
    spk::ptr_t<spk::text_t> text;

    {
        smells_blood_id = spk::music_create("smells_blood.mp3");
        coin_sound_id   = spk::chunk_create("coin_sound.wav");
        // spk::music_play(smells_blood_id, 1);

        font_id = spk::font_create("./Raleway-Regular.ttf", 128, 22);

        sprite_array_id = spk::sprite_array_create(32, 32, 5);
        spk::sprite_array_set(sprite_array_id, "./texture_array/image1.png", 0);
        spk::sprite_array_set(sprite_array_id, "./texture_array/image2.png", 1);
        spk::sprite_array_set(sprite_array_id, "./texture_array/image3.png", 2);

        high_def_array = spk::sprite_array_create(1000, 1000, 2);
        spk::sprite_array_set(high_def_array, "./texture_array/windows_logo.png", 0);

        spk::tile_dictionary_t& td = resources.tile_dictionary;
        
        td[1].sprite.id   = sprite_array_id;
        td[1].sprite.z    = -5.0f;
        td[1].restitution = 1.1f;
        td[1].friction = 0.0f;

        td[2].sprite.id = sprite_array_id;
        td[2].sprite.index = 1;
        td[2].sprite.z = 1.0f;
        td[2].density = 50.0f;
        td[2].friction = 0.0f;
        td[2].restitution = 0.0f;

        td[4].sprite.id = sprite_array_id;
        td[2].sprite.z = 1.0f;
        td[4].sprite.index = 2;
        td[4].density = -5.0f;
    }

    { // canvas 
        canvas.font = font_id;

        text = canvas.element<spk::text_t>();

        text->x_set(spk::constraint_relative(0.2f));
        text->y_set(spk::constraint_relative(0.95f));
        text->width_set(spk::constraint_relative(0.2f));
        text->height_set(spk::constraint_relative(0.05f));
        text->text       = "loading..";
        text->text_color = {1.0f, 1.0f, 1.0f};
        text->color      = {0.0f, 0.0f, 0.0f, 0.0f};

        spk::settings().target_tps = 60;
        spk::settings().target_fps = 120;

    }

    for(size_t i = 0; i < 100; i++) {
        scene.ecs_world.entity().set([&](spk::comp_rigid_body_t& rb, spk::comp_sprite_t& sprite, spk::comp_particles_t& ps) {
            float random = rand();
            random *= 0.01f;

            rb->SetType(b2_dynamicBody);
            rb->SetBullet(true);
            rb->SetTransform((glm::vec2){fmod(random, 20.0f) - 10.0f, fmod(random, 20.0f) - 10.0f}, 0.0f);

            b2PolygonShape shape;
            float hl = fmod(random, 2.5f) + 0.4f;
            shape.SetAsBox(hl, hl);

            b2FixtureDef fdef;
            fdef.shape = &shape;
            fdef.density = 0.5f;
            fdef.restitution = 0.1f;
            rb->CreateFixture(&fdef);

            sprite.size = {hl, hl};
            sprite.id = 1;
            sprite.index = 1;
            sprite.z = 1.0f;

            ps.funnel = spk::PARTICLES_FUNNEL_FUNNEL;
            ps.step   = 0.2f; // fun :)
            ps.length = 1.0f;
            ps.base_speed = 9.0f;
            ps.sprite = 4;
            ps.base_cycle = 0.5f;
            ps.base_lifetime = 1.0f;
            ps.max = UINT32_MAX;
            ps.flags  = spk::PARTICLES_FLAGS_WORLD_DIRECTION | spk::PARTICLES_FLAGS_WORLD_POSITION | spk::PARTICLES_FLAGS_ACTIVE;
        });
    }
    
    flecs::entity wall = scene.ecs_world.entity().set([&](spk::comp_rigid_body_t& rb, spk::comp_tilemap_t& tm){
        rb->SetType(b2_staticBody);

        for(uint32_t x = 0; x < tm.tiles.get_width(); x++) {
            for(uint32_t y = 0; y < tm.tiles.get_height(); y++) {
                if(x != 0 && x != (tm.tiles.get_width() - 1) &&
                    y != 0 && y != (tm.tiles.get_height() - 1)) {
                    tm.tiles.get(x, y).id = 1;
                    tm.tiles.get(x, y).flags = 0;
                } else {
                    tm.tiles.get(x, y).id = 2;
                    tm.tiles.get(x, y).flags = spk::TILE_FLAGS_COLLIADABLE;
                }
            }
        }

        tm.add_fixtures(rb);
    });

    flecs::entity character = scene.ecs_world.entity().set([&](
            spk::comp_rigid_body_t& rb, 
            spk::comp_character_controller_t& cc, 
            spk::comp_contact_callback_t& callbacks,
            spk::comp_tilemap_t& tilemap) {
        rb->SetType(b2_dynamicBody);
        rb->SetBullet(true);

        tilemap.tiles.get(1, 0) = 2;
        tilemap.tiles.get(1, 1) = 2;
        tilemap.tiles.get(1, 2) = 2;
        tilemap.tiles.get(1, 3) = 2;
        tilemap.tiles.get(1, 4) = 2;
        tilemap.tiles.get(1, 5) = 2;
        tilemap.tiles.get(1, 6) = 2;
        tilemap.tiles.get(1, 7) = 2;
        tilemap.tiles.get(1, 8) = 2;
        tilemap.tiles.get(1, 9) = 2;
        tilemap.tiles.get(1, 10) = 2;
        tilemap.tiles.get(1, 11) = 2;

        tilemap.add_fixtures(rb);

        cc.speed = 100.0f;

        b2CircleShape shape;
        shape.m_radius = 15.0f;
        b2FixtureDef fixture;
        fixture.shape = &shape;
        fixture.isSensor = true;
        rb->CreateFixture(&fixture);
    
        callbacks.end = [&](flecs::entity self, flecs::entity other, b2Contact* contact) {
            spk::chunk_play(coin_sound_id, 0);

            if(other == wall)
                return;
            
            b2Fixture* self_fixture;
            b2Fixture* other_fixture;

            if(self == contact->GetEntityA()) {
                self_fixture = contact->GetFixtureA();
                other_fixture = contact->GetFixtureB();
            } else {
                self_fixture = contact->GetFixtureB();
                other_fixture = contact->GetFixtureA();
            }

            if(!self_fixture->IsSensor())
                return;

            {
                glm::vec2 dir_away = 
                    glm::normalize((glm::vec2)other_fixture->GetBody()->GetPosition() - (glm::vec2)self_fixture->GetBody()->GetPosition());

                float strength = 10.0f;

                other_fixture->GetBody()->ApplyLinearImpulseToCenter(-dir_away * strength, true);
            }
        };
    }).add<spk::tag_body_render_t>();

    scene.camera.get_ref<spk::comp_camera_t>()->scale = 1.5f;

    scene.user_data.update = [&](){
        character.set([&](spk::comp_rigid_body_t& rb){
            scene.camera.get_ref<spk::comp_camera_t>()->pos = rb->GetPosition();

            rb->ApplyTorque(100000, true);
        });

        text->text = std::string(spk::build_name()) + " | FPS:" + std::to_string(spk::statistics().fps) + " | TPS:"  + std::to_string(spk::statistics().tps);
    };  

    code = spk::run();

    spk::free();

    return code;
}