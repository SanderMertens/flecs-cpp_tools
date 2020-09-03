#include <observer.h>
#include <iostream>

struct Position {
    float x;
    float y;
};

int main(int argc, char *argv[]) {
    flecs::world ecs;

    ecs.import<flecs::observable>();

    // Create observable entity
    auto e1 = ecs.entity("e1");
    auto e2 = ecs.entity("e2");
    auto e3 = ecs.entity("e3");

    // Create observer
    flecs::observer<Position> observer([](flecs::entity e, const Position& p) {
        std::cout << "Entity " << e.name() 
            << ": {" << p.x << ", " << p.y << "}" << std::endl;
    });

    // Listen to observable
    observer.observe(e1);
    observer.observe(e2);

    // Trigger observer
    e1.set<Position>({10, 20});
    e2.set<Position>({30, 40});

    // This won't trigger the observer
    e3.set<Position>({50, 60});

    // Unobserve e2
    observer.unobserve(e2);

    // Will no longer trigger observer
    e2.set<Position>({70, 80});
}
