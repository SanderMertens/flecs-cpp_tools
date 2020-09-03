# flecs-cpp_tools
This repository contains a collection of useful tools built on top of the public Flecs API.

## Building
The library is header-only. Just copy the headers of the features you need to your project.

## Observer
Observers allow an application to subscribe to component updates of specific entities.

```cpp
// Create observer
flecs::observer<Position> observer([](flecs::entity e, const Position& p) {
    std::cout << e.name() << ": {" << p.x << ", " << p.y << "}" << std::endl;
});

// Listen to updates for Position
observer.observe(e1);
observer.observe(e2);

// Trigger observer
e1.set<Position>({10, 20});
```

## Timers
Timers execute an action after a certain period has expired
```cpp
// Add Velocity after 5 seconds
e.set_trait<flecs::AddTimer, Velocity>({5});

// Delete Position after 10 seconds
e.set_trait<flecs::RemoveTimer, Position>({10});

// Delete entity after 15 seconds
e.set<flecs::DeleteTimer>({15});
```

## Dump
Dump is a utility that prints information about an entity to the console

```cpp
auto Thing = ecs.entity("Thing")
    .set<Mass>({100});

auto Animal = ecs.entity("Animal")
    .add_instanceof(Thing);  

auto Dog = ecs.entity("Dog")
    .add_instanceof(Animal);           

auto e1 = ecs.entity("Beethoven")
    .add_instanceof(Dog)
    .set<Position>({10, 20})
    .set<Velocity>({1, 2});

// Dump entity
flecs::dump(e1);
```
Output:
```
====================================
 Beethoven
------------------------------------
  - Instanceof | Dog
    - Instanceof | Animal
      - Instanceof | Thing
        - Mass
        - Name
      - Name
    - Name
  - Velocity
  - Position
  - Name
------------------------------------
```
