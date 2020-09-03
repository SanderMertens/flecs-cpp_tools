#ifndef FLECS_OBSERVABLE_H
#define FLECS_OBSERVABLE_H

#include "flecs-cpp_tools/bake_config.h"
#include <unordered_map>
#include <functional>

namespace flecs {

// Observer callback type
template <typename T>
using observer_func = std::function<void(flecs::entity, const T&)>;

// Observer context data, responsible for reintroducing type safety
template <typename T>
class observer_invoker {
public:
    observer_invoker(const observer_func<T>& func) : m_func(func), m_id(0) { }

    void id(entity_t id) {
        m_id = id;
    }

    entity_t id() const {
        return m_id;
    }

    // Static function that can be stored in observer data
    static void invoke(flecs::entity e, void *ptr, void *ctx) {
        // Instance of self is stored in observer context
        observer_invoker<T> *self = static_cast<observer_invoker<T>*>(ctx);
        self->m_func(e, static_cast<T*>(ptr)[0]);
    }
private:
    const observer_func<T>& m_func;
    entity_t m_id;
};

// Observer data that is stored in list of observers
struct observer_data {
    void *ctx;
    void(*invoke)(flecs::entity e, void *ptr, void *ctx);
};

// Trait that stores a list of observers
struct Observable {
    std::unordered_map<entity_t, observer_data> observers;
};

// Typed observer class which allows for observing multiple entities
template<typename T>
class observer final {
public:
    observer(observer_func<T> func) 
        : m_func(func)
        , m_invoker(new observer_invoker<T>(m_func)) { }

    ~observer() {
        delete m_invoker;
    }

    void observe(flecs::entity observable) {
        if (!m_invoker->id()) {
            // Create a unique id for the observer, so we can store it in a map.
            // Register the id with the invoker, so that the observer object can
            // be moved around on the stack.
            m_invoker->id(ecs_new_id(observable.world().c_ptr()));
        }

        // Create the observer data that will be added to the list of observers
        // of the observable.
        observer_data data;
        data.ctx = m_invoker;
        data.invoke = observer_invoker<T>::invoke;
        
        // Add the Observable trait for the type of the observer
        Observable *o = observable.get_trait_mut<Observable, T>();

        // Add the observer to the list
        o->observers[m_invoker->id()] = data;
    }

    void unobserve(flecs::entity observable) {
        entity_t id = m_invoker->id();
        if (id) {
            Observable *o = observable.get_trait_mut<Observable, T>();
            o->observers.erase(id);

            // If observable has no more observers, remove trait
            if (!o->observers.size()) {
                observable.remove_trait<Observable, T>();
            }
        }
    }

private:
    const observer_func<T> m_func;
    observer_invoker<T>* m_invoker;
};

// Module implementation
class observable {
public:
    observable(flecs::world& ecs) {
        ecs.module<flecs::observable>();

        // Register component so it can be accessed by name in signature
        ecs.component<Observable>();

        // Invoke observers when component is set.
        // This system subscribes for the Observable trait which stores the list
        // of observers, in addition to the component to which the trait is
        // applied (the wildcard). The latter needs to be part of the signature
        // or the system would not trigger when the component is set. 
        //
        // By using traits instead of a regular OnSet system we ensure that only
        // entities with the Observable trait trigger the system. Without the
        // trait, updates from any entity would trigger the system.
        ecs.system<>(nullptr, "TRAIT | Observable, TRAIT | Observable > *")
            .kind(flecs::OnSet)
            .action([](flecs::iter it) {
                // List of observers
                auto observables = it.column<Observable>(1);

                // The component data. Since we don't know the type of the 
                // component at compile time, we need to use an untyped column.
                auto data = it.column(2);

                // It is possible that multiple observable entities were updates
                for (auto i : it) {
                    // Iterate observers, pass data to each one
                    for (auto observer : observables[i].observers) {
                        observer.second.invoke( it.entity(i), data[i], observer.second.ctx ); 
                    }
                }
            });
    }
};

}

#endif
