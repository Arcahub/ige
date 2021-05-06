#include "ige.hpp"
#include <iostream>

using ige::core::App;
using ige::core::State;
using ige::ecs::Schedule;
using ige::game::PerspectiveCamera;
using ige::game::Transform;
using ige::game::WindowSettings;
using ige::math::Vec3;

class RootState : public State {
    int counter = 0;

    void on_start(App& app) override
    {
        auto camera = app.world().create_entity();

        camera.add_component(Transform::look_at(Vec3(3.0f), Vec3(0.0f)));
        camera.add_component(PerspectiveCamera(90.0f));
    }

    void on_update(App& app) override
    {
        counter++;

        std::cout << "Hello " << counter << std::endl;

        if (counter >= 10) {
            app.quit();
        }
    }
};

int main()
{
    App::Builder()
        .insert(WindowSettings { "Hello, World!", 800, 600 })
        .run<RootState>();
}
