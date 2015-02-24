#pragma once

#include <stdexcept>
#include <utility>

// An error thrown at runtime when cases that require special-cased behavior are detected...
//  but there is currently no compelling reason to worry about these cases.
struct LazyProgrammerError : public std::runtime_error {
	template <class... Args>
	LazyProgrammerError(Args&&... args)
	: std::runtime_error(std::forward<Args>(args)...)
	{ }
};
