#ifndef SPLIT_H
#define SPLIT_H

#include <string>
#include <algorithm>
#include <vector>

namespace cppext
{
    template <typename Iter, typename StringT>
    auto findFirstNotOf(Iter&& begin, Iter&& end, StringT&& delim)
        -> decltype(auto)
    {
        return std::find_first_of(
            std::forward<Iter>(begin),
            std::forward<Iter>(end),
            std::cbegin(delim),
            std::cend(delim),
            [](auto a, auto b) { return a != b; }
        );
    }

    template <typename Iter, typename StringT>
    auto findFirstOf(Iter&& begin, Iter&& end, StringT&& delim)
        -> decltype(auto)
    {
        return std::find_first_of(
            std::forward<Iter>(begin),
            std::forward<Iter>(end),
            std::cbegin(delim),
            std::cend(delim)
        );
    }

    template <class...>
    using void_t = void;

    template <class, class T, class... Args>
    struct is_brace_constructible_ : std::false_type {};

    template <class T, class... Args>
    struct is_brace_constructible_ <
        void_t<decltype(T{ std::declval<Args>()... }) > ,
        T, Args... > : std::true_type {};

    template <class T, class... Args>
    using is_brace_constructible = is_brace_constructible_<void_t<>, T, Args...>;

    template<typename StringT1, typename StringT2, typename Container = std::vector<std::decay_t<StringT1>>>
    auto split(StringT1&& toSplit, StringT2&& delim)
        ->std::enable_if_t
        <
        (!std::is_same_v<std::decay_t<StringT1>, std::decay_t<StringT2>>)
        && is_brace_constructible<std::decay_t<StringT1>, std::decay_t<StringT2>>::value,
        Container
        >
    {
        return split<StringT1, std::add_rvalue_reference_t<std::decay_t<StringT1>>, Container>(std::forward<StringT1>(toSplit), std::decay_t<StringT1>{delim});
    }

    template<typename StringT1, typename StringT2, typename Container = std::vector<std::decay_t<StringT2>>>
    auto split(StringT1&& toSplit, StringT2&& delim)
        ->std::enable_if_t
        <
        (!std::is_same_v < std::decay_t<StringT1>, std::decay_t<StringT2>>)
        && is_brace_constructible<std::decay_t<StringT2>, std::decay_t<StringT1>>::value,
        Container
        >
    {
        return split<std::add_rvalue_reference_t<std::decay_t<StringT2>>, StringT2, Container>(std::decay_t<StringT2>{toSplit}, std::forward<StringT2>(delim));
    }

    template<typename CharT, typename Container = std::vector<std::basic_string<CharT>>>
    auto split(const CharT* toSplit, CharT delim)
        -> Container
    {
        return split<std::add_rvalue_reference_t<std::basic_string<CharT>>, std::add_rvalue_reference_t<std::basic_string<CharT>>, Container>(std::basic_string<CharT>{toSplit}, std::basic_string<CharT>{delim});
    }

    template<typename CharT, typename Container = std::vector<std::basic_string<CharT>>>
    auto split(const CharT* toSplit, const CharT* delim)
        -> Container
    {
        return split<std::add_rvalue_reference_t<std::basic_string<CharT>>, std::add_rvalue_reference_t<std::basic_string<CharT>>, Container>(std::basic_string<CharT>{toSplit}, std::basic_string<CharT>{delim});
    }

    template<typename StringT1, typename StringT2, typename Container = std::vector<std::decay_t<StringT1>>>
    auto split(StringT1&& toSplit, StringT2&& delim)
        ->std::enable_if_t
        <
        std::is_same_v<std::decay_t<StringT1>, std::decay_t<StringT2>>,
        Container
        >
    {
        static_assert(std::is_same_v<std::decay_t<StringT1>, typename Container::value_type>, "Container value type should be same as first of the split arguments.");

        auto store = Container{};

        auto begin = std::cbegin(toSplit);
        auto end = std::cend(toSplit);
        while(true)
        {
            auto beginCopy = findFirstNotOf(begin, end, delim);
            if(beginCopy == end)
            {
                return store;
            }
            auto endCopy = findFirstOf(beginCopy, end, delim);
            store.emplace_back(beginCopy, endCopy);
            begin = endCopy;
        }
    }
}

#endif
