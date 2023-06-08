/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * log
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifndef G_UTILS_LOG_H
#define G_UTILS_LOG_H

#include "core/const.h"
#include "utils/fs.h"
#include <fmt/core.h>
#include <fstream>
#include <string>
#include <type_traits>
#include <utility>

#ifdef G_DEBUG_MODE
#define G_DEBUG(f, ...) \
	std::cerr << __FILE__ << "::" << __func__ << "() - " << fmt::format(f __VA_OPT__(, ) __VA_ARGS__) << "\n";
#else
#define G_DEBUG(f, ...) \
	do                  \
	{                   \
	} while (0)
#endif

namespace giada::u::log
{
inline std::ofstream file;
inline int           mode;

/* init
Initializes logger. Mode defines where to write the output: LOG_MODE_STDOUT,
LOG_MODE_FILE and LOG_MODE_MUTE. */

int init(int mode);

void close();

/* string_to_c_str
Internal utility function for string transformation. Uses forwarding references
(&&) to avoid useless string copy. */

static constexpr auto string_to_c_str = [](auto&& s) {
	/* Remove any reference and const-ness, since the function can handle 
	l-value and r-value, const or not. TODO - Use std::remove_cvref instead, 
	when switching to C++20. */
	if constexpr (std::is_same_v<std::remove_const_t<std::remove_reference_t<
	                                 decltype(s)>>,
	                  std::string>)
		// If the argument is a std::string return an old-style C-string
		return s.c_str();
	else
		// Return the argument unchanged otherwise
		return s;
};

/* print
A variadic printf-like logging function. Any `std::string` argument will be 
automatically transformed into a C-string. */

template <typename... Args>
static void print(const char* format, Args&&... args)
{
	if (mode == LOG_MODE_MUTE)
		return;

	if (mode == LOG_MODE_FILE && file.is_open())
	{
		// Ugly temporary workaround. Will be fixed in later commits when moving to fmt

		// Compute size first for the holding vector<char>
		const int         size = std::snprintf(nullptr, 0, format, string_to_c_str(std::forward<Args>(args))...);
		std::vector<char> buf(size + 1); // Note +1 for null terminator

		// Write to temp buffer
		std::sprintf(&buf[0], format, string_to_c_str(std::forward<Args>(args))...);

		// Write temp buffer to fstream
		file.write(&buf[0], buf.size() - 1);
#ifdef _WIN32
		fflush(f);
#endif
	}
	else
		std::printf(format, string_to_c_str(std::forward<Args>(args))...);
}
} // namespace giada::u::log

#endif
