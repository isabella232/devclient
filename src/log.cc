//
// Created by jakub on 25.10.2019.
//

#include <fmt/format.h>
#include <log.hh>

void Logger::log(const char *level, const char *fmt, fmt::format_args args)
{
	fmt::print("{}: ", level);
	fmt::vprint(fmt, args);
	fmt::print("\n");
}
