#pragma once

#include <cstdlib>
#include <type_traits>

template <typename T>
  requires std::is_integral_v<T>
constexpr T div_ceil(T a, T b) noexcept {
  auto div_result = std::div(a, b);
  if constexpr (std::is_unsigned_v<T>)
    return div_result.quot + (div_result.rem != 0);
  else
    return div_result.quot + (div_result.rem != 0 && ((a < 0) == (b < 0)));
}
