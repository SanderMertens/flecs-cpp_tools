#ifndef FLECS_OBSERVABLE_H
#define FLECS_OBSERVABLE_H

#include "flecs-cpp_tools/bake_config.h"
#include <vector>
#include <functional>

namespace flecs {

/** Observer callback type */
template <typename T>
using observer_func = std::function<void(flecs::entity, const T&)>;

/** Observer context data, responsible for reintroducing type safety */
template <typename T>
class observer_invoker {
public:
    observer_invoker(observer_func<T>& func) : m_func(func) { }

    /* Static function that can be stored in observer data */
    static void invoke(flecs::entity e, void *ptr, void *ctx) {
        /* Instance of self is stored in observer context */
        observer_invoker<T> *self = static_cast<observer_invoker<T>*>(ctx);
        self->m_func(e, static_cast<T*>(ptr)[0]);
    }
private:
    observer_func<T>& m_func;
};

/** Observer data that is stored in list of observers */
struct observer_data {
    void *ctx;
    void(*invoke)(flecs::entity e, void *ptr, void *ctx);
};

/** Trait that stores a list of observers */
struct Observable {
    std::vector<observer_data> observers;
};

/** Typed observer class which allows for observing multiple entities */
template<typename T>
class observer final {
public:
    observer(observer_func<T> func) 
        : m_invoker(new observer_invoker<T>(func)) { }

    ~observer() {
        delete m_invoker;
    }

    void observe(flecs::entity observable) {
        observer_data data;
        data.ctx = m_invoker;
        data.invoke = observer_invoker<T>::invoke;
        
        Observable *o = observable.get_trait_mut<Observable, T>();
        o->observers.push_back(data);
    }

private:
    observer_invoker<T>* m_invoker;
};


/** Module implementation */
class observable {
public:
    observable(flecs::world& ecs) {
        ecs.module<flecs::observable>();

        /** Register component so it can be accessed by name in signature */
        ecs.component<Observable>();

        /** Invoke observers when component is set */
        ecs.system<>(nullptr, "TRAIT | Observable, TRAIT | Observable > *")
            .kind(flecs::OnSet)
            .action([](flecs::iter it) {
                auto observables = it.column<Observable>(1);
                auto data = it.column(2);

                for (auto i : it) {
                    for (auto observer : observables[i].observers) {
                        observer.invoke(
                            it.entity(i),
                            data[i],
                            observer.ctx
                        );
                    }
                }
            });
    }
};

}

#endif