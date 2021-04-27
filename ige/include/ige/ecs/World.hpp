/*
** EPITECH PROJECT, 2021
** ige
** File description:
** World
*/

#ifndef B1C7D01B_EFEC_457F_B2AD_A4234DA7F87A
#define B1C7D01B_EFEC_457F_B2AD_A4234DA7F87A

#include "ige/core/Any.hpp"
#include "ige/ecs/MapStorage.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace ige {
namespace ecs {

    namespace impl {
        using TypeId = std::size_t;

        template <typename T> struct type_id_ptr {
            static const T* const id;
        };

        template <typename T> const T* const type_id_ptr<T>::id = nullptr;

        template <typename T> constexpr TypeId type_id() noexcept
        {
            return reinterpret_cast<TypeId>(&type_id_ptr<T>::id);
        }
    }

    using EntityId = std::uint64_t;

    template <typename Component> struct ComponentStorage {
        using Type = MapStorage<EntityId, Component>;
    };

    class World {
    public:
        class Entity {
        public:
            Entity(World&, EntityId);
            Entity(const Entity&) = default;

            bool operator==(const Entity&) const;
            bool operator!=(const Entity&) const;
            bool remove();

            template <typename T, typename... Args,
                typename = std::enable_if_t<std::is_object<T>::value
                    && std::is_constructible<T, Args...>::value>>
            T& add_component(Args&&... args)
            {
                return m_wld.add_component<T>(
                    m_id, std::forward<Args>(args)...);
            }

            template <typename T,
                typename = std::enable_if_t<std::is_object<T>::value>>
            std::optional<std::reference_wrapper<T>> get_component()
            {
                return m_wld.get_component<T>(m_id);
            }

            template <typename T,
                typename = std::enable_if_t<std::is_object<T>::value>>
            std::optional<std::reference_wrapper<const T>> get_component() const
            {
                return m_wld.get_component<T>(m_id);
            }

            template <typename T,
                typename = std::enable_if_t<std::is_object<T>::value>>
            std::optional<T> remove_component()
            {
                return m_wld.remove_component<T>(m_id);
            }

            EntityId id()
            {
                return m_id;
            }

        private:
            World& m_wld;
            EntityId m_id;
        };

        Entity create_entity();

        template <typename T, typename... Args,
            typename = std::enable_if_t<std::is_object<T>::value
                && std::is_constructible<T, Args...>::value>>
        T& set(Args&&... args)
        {
            impl::TypeId id = impl::type_id<T>();

            return set_any(id, core::Any::from<T>(std::forward<Args>(args)...))
                .template as<T>();
        }

        template <typename T,
            typename = std::enable_if_t<std::is_object<T>::value>>
        std::optional<std::reference_wrapper<T>> get()
        {
            impl::TypeId id = impl::type_id<T>();

            if (auto any = get_any(id)) {
                return { any->get().template as<T>() };
            } else {
                return {};
            }
        }

        template <typename T,
            typename = std::enable_if_t<std::is_object<T>::value>>
        std::optional<std::reference_wrapper<const T>> get() const
        {
            impl::TypeId id = impl::type_id<T>();

            if (auto any = get_any(id)) {
                return { any->get().template as<T>() };
            } else {
                return {};
            }
        }

        template <typename T,
            typename = std::enable_if_t<std::is_object<T>::value>>
        std::optional<T> remove()
        {
            return std::move(
                remove_any(impl::type_id<T>()).map([](core::Any any) {
                    return std::move(any.template as<T>());
                }));
        }

        bool remove_entity(EntityId);

        template <typename T, typename... Args,
            typename = std::enable_if_t<std::is_object<T>::value
                && std::is_constructible<T, Args...>::value>>
        T& add_component(EntityId ent, Args&&... args)
        {
            using Storage = typename ComponentStorage<T>::Type;

            auto optstrg = get<Storage>();

            if (!get<Storage>().has_value()) {
                set<Storage>();
            }

            auto& strg = get<Storage>()->get();

            strg.set(std::move(ent), std::forward<Args>(args)...);

            if (m_components.find(impl::type_id<T>()) == m_components.end()) {
                m_components.emplace(impl::type_id<T>(), [&](EntityId ent) {
                    return remove_component<T>(ent).has_value();
                });
            }

            return strg.get(ent)->get();
        }

        template <typename T,
            typename = std::enable_if_t<std::is_object<T>::value>>
        std::optional<std::reference_wrapper<T>> get_component(EntityId ent)
        {
            using Storage = typename ComponentStorage<T>::Type;

            if (auto strg = get<Storage>()) {
                return strg->get().get(ent);
            } else {
                return {};
            }
        }

        template <typename T,
            typename = std::enable_if_t<std::is_object<T>::value>>
        std::optional<std::reference_wrapper<const T>> get_component(
            EntityId ent) const
        {
            using Storage = typename ComponentStorage<T>::Type;

            if (auto strg = get<Storage>()) {
                return strg->get().get(ent);
            } else {
                return {};
            }
        }

        template <typename T,
            typename = std::enable_if_t<std::is_object<T>::value>>
        std::optional<T> remove_component(EntityId ent)
        {
            using Storage = typename ComponentStorage<T>::Type;

            if (auto strg = get<Storage>()) {
                return strg->get().remove(ent);
            } else {
                return {};
            }
        }

    private:
        core::Any& set_any(impl::TypeId id, core::Any any)
        {
            m_resources.erase(id);
            m_resources.insert(std::make_pair(id, std::move(any)));
            return m_resources.at(id);
        }

        std::optional<std::reference_wrapper<core::Any>> get_any(
            impl::TypeId id)
        {
            auto it = m_resources.find(id);

            if (it != m_resources.end()) {
                return { it->second };
            } else {
                return {};
            }
        }

        std::optional<std::reference_wrapper<const core::Any>> get_any(
            impl::TypeId id) const
        {
            auto it = m_resources.find(id);

            if (it != m_resources.end()) {
                return { it->second };
            } else {
                return {};
            }
        }

        std::optional<core::Any> remove_any(impl::TypeId id)
        {
            auto it = m_resources.find(id);

            if (it != m_resources.end()) {
                auto any = std::move(it->second);
                m_resources.erase(id);
                return { std::move(any) };
            } else {
                return {};
            }
        }

        using CompRemover = std::function<bool(EntityId)>;

        EntityId m_last_entity = 0;
        std::unordered_map<impl::TypeId, CompRemover> m_components;
        std::unordered_map<impl::TypeId, core::Any> m_resources;
    };

}
}

#endif /* B1C7D01B_EFEC_457F_B2AD_A4234DA7F87A */
