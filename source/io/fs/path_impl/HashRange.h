/*----------------------------------------------------------------------------------------------
 *  Copyright (c) 2020 - present Alexander Voitenko
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *----------------------------------------------------------------------------------------------*/

#pragma once

#include "Export.h"

#include <cstdint>
#include <cstddef>
#include <functional>

namespace tarm {
namespace io {
namespace fs {

IO_DLL_PUBLIC
void hash_combine_impl(std::size_t& h, std::uint64_t k);

template <class T>
inline void hash_combine(std::size_t& seed, T const& v) {
    std::hash<T> hasher;
    return hash_combine_impl(seed, hasher(v));
}

template <class It>
inline std::size_t hash_range(It first, It last) {
    std::size_t seed = 0;

    for(; first != last; ++first) {
        hash_combine(seed, *first);
    }

    return seed;
}

template <class It>
inline void hash_range(std::size_t& seed, It first, It last) {
    for(; first != last; ++first) {
        hash_combine(seed, *first);
    }
}

} // namespace fs
} // namespace io
} // namespace tarm
