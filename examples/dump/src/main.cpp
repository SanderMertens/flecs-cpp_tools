#include <dump.h>
#include <iostream>

struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

struct Mass {
    float value;
};

int main(int argc, char *argv[]) {
    flecs::world ecs;

    ecs.import<flecs::timers>();

    auto component = ecs.component<Position>();

    auto system = ecs.system<Position, const Velocity>("Move")
        .action([](flecs::iter it, 
            flecs::column<Position> p, 
            flecs::column<const Velocity> v) { 
                // Dump iterator
                flecs::dump(it);
            });

    auto parent = ecs.entity("Parent");

    auto Thing = ecs.entity("Thing")
        .set<Mass>({100});

    auto Animal = ecs.entity("Animal")
        .add_instanceof(Thing);  

    auto Dog = ecs.entity("Dog")
        .add_instanceof(Animal);           

    auto e1 = ecs.entity("Beethoven")
        .add_childof(parent)
        .add_instanceof(Dog)
        .set<Position>({10, 20})
        .set<Velocity>({1, 2})
        .set_trait<flecs::RemoveTimer, Position>({10});

    // Dump entity
    flecs::dump(e1);

    // Dump component
    flecs::dump(component);

    // Dump system
    flecs::dump(system);

    // Progress world, will dump iterator
    ecs.progress();
}
