#pragma once

#include <algorithm>

template <typename Container, typename UnaryPredicate>
[[nodiscard]] auto constexpr count_if(Container& c, UnaryPredicate const& p) {
    return std::count_if(std::begin(c), std::end(c), p);
}

template <typename Container, typename UnaryPredicate>
[[nodiscard]] auto constexpr all_of(Container& c, UnaryPredicate const& p) {
    return std::all_of(std::begin(c), std::end(c), p);
}

template <typename Container, typename UnaryPredicate>
[[nodiscard]] auto constexpr any_of(Container& c, UnaryPredicate const& p) {
    return std::any_of(std::begin(c), std::end(c), p);
}

template <typename Container, typename URBG>
auto shuffle(Container& c, URBG&& g) {
    return std::shuffle(std::begin(c), std::end(c), g);
}
