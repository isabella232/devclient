//
// Created by jakub on 24.10.2019.
//

#ifndef DEVCLIENT_UTILS_HH
#define DEVCLIENT_UTILS_HH

#include <fmt/format.h>
#include <glibmm.h>
#include <string>

template <>
struct fmt::formatter<Glib::ustring> {
	template <typename ParseContext>
	constexpr auto parse(ParseContext &ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(const Glib::ustring &s, FormatContext &ctx)
	{
		return format_to(ctx.out(), "{}", s.c_str());
	}
};

#endif //DEVCLIENT_UTILS_HH
