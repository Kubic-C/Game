#pragma once
#include "quad.hpp"

namespace spk {
    struct render_scene_tt {
        quad_scene_tt quad_batch;
    };

    class renderer2D_tt : public system_tt {
    public:
        renderer2D_tt();

        void init(scene_tt& scene, void* data);
        void update(scene_tt& scene, float deltatime);
        void tick(scene_tt& scene, float deltatime) {}
        static void resize(void* self, int width, int height);
        void free();

        struct vertex {
            float x, y, z;
            uint32_t agbr;
        };

        render_system_manager_tt renderer_manager;

    private:
        // need a place to access each render system's
        // non-base-class methods
        quad_renderer_tt quad_renderer;
    };
}