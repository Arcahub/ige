/*
** EPITECH PROJECT, 2021
** ige
** File description:
** VecStorage
*/

#ifndef DB322AB1_C834_42D3_82D6_5F31BCC83A33
#define DB322AB1_C834_42D3_82D6_5F31BCC83A33

#include <functional>
#include <optional>
#include <vector>

namespace ige {
namespace ecs {

    template <typename T> class VecStorage {
    public:
        VecStorage() = default;
        virtual ~VecStorage() = default;

        VecStorage(VecStorage&& other)
            : m_data(std::move(other.m_data))
        {
        }

        template <typename... Args,
            typename
            = std::enable_if_t<std::is_constructible<T, Args...>::value>>
        void set(std::size_t idx, Args&&... args)
        {
            std::optional<T> opt;

            opt.emplace(std::forward<Args>(args)...);

            m_data.resize(idx + 1);
            m_data.emplace(m_data.begin() + idx, std::move(opt));
        }

        std::optional<std::reference_wrapper<const T>> get(
            const std::size_t& idx) const
        {
            std::optional<std::reference_wrapper<const T>> element;

            if (idx < m_data.size()) {
                if (auto value = m_data[idx]) {
                    element.emplace(*value);
                }
            }

            return element;
        }

        std::optional<std::reference_wrapper<T>> get(const std::size_t& idx)
        {
            std::optional<std::reference_wrapper<T>> element;

            if (idx < m_data.size()) {
                if (auto value = m_data[idx]) {
                    element.emplace(*value);
                }
            }

            return element;
        }

        std::optional<T> remove(const std::size_t& idx)
        {
            std::optional<T> element;

            if (idx < m_data.size()) {
                m_data[idx].swap(element);
            }

            return element;
        }

    private:
        std::vector<std::optional<T>> m_data;
    };

}
}

#endif /* DB322AB1_C834_42D3_82D6_5F31BCC83A33 */
