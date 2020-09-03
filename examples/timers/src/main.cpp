#include <timers.h>
#include <iostream>

struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

int main(int argc, char *argv[]) {
    flecs::world ecs;
    
    ecs.import<flecs::timers>();

    // Create entity with two components
    auto e = ecs.entity()
        .set<Position>({10, 20});

    // Add Velocity after 5 seconds
    e.set_trait<flecs::AddTimer, Velocity>({5});

    // Delete Position after 10 seconds
    e.set_trait<flecs::RemoveTimer, Position>({10});

    // Delete entity after 15 seconds
    e.set<flecs::DeleteTimer>({15});

    // Run main loop at 1 FPS
    ecs.set_target_fps(1);

    while (ecs.progress()) { 
        // Print type of entity so we can see the effect of the timers
        std::cout << e.type().str() << std::endl;
    }
}
