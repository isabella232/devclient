//
// Created by jakub on 25.10.2019.
//

#ifndef DEVCLIENT_LOG_HH
#define DEVCLIENT_LOG_HH

#include <fmt/format.h>

class Logger
{
public:
	template <typename... Args>
	static void debug(const char *fmt, const Args &... args)
	{
		Logger::log("DEBUG", fmt, fmt::make_format_args(args...));
	}

	template <typename... Args>
	static void info(const char *fmt, const Args &... args)
	{
		Logger::log("INFO", fmt, fmt::make_format_args(args...));
	}

	template <typename... Args>
	static void warning(const char *fmt, const Args &... args)
	{
		Logger::log("WARNING", fmt, fmt::make_format_args(args...));
	}

	template <typename... Args>
	static void error(const char *fmt, const Args &... args)
	{
		Logger::log("ERROR", fmt, fmt::make_format_args(args...));
	}

	static void log(const char *level, const char *fmt,
	    fmt::format_args args);
};

#endif //DEVCLIENT_LOG_HH
