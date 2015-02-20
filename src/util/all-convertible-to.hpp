#pragma once

#include <type_traits>

//------------------------------------------------
// (pay no attention to the man behind the curtain)

template <bool B, class Desired, class... Ts>
struct all_convertible_to_impl
{ };

// Failure base case:  first arg is false
template <class Desired, class... Ts>
struct all_convertible_to_impl<false, Desired, Ts...> {
	static constexpr bool value = false;
};

// Success base case:  no types left
template <class Desired>
struct all_convertible_to_impl<true, Desired> {
	static constexpr bool value = true;
};

// Recursive case
template <class Desired, class T, class... Rest>
struct all_convertible_to_impl<true, Desired, T, Rest...> {
	static constexpr bool value = all_convertible_to_impl <
		std::is_convertible<T, Desired>::value,
		Desired, Rest...
	>::value;
};

//------------------------------------------------

template <class Desired, class... Ts>
struct all_convertible_to {
	static constexpr bool value = all_convertible_to_impl <
		true,
		Desired, Ts...
	>::value;
};

