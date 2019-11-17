//
// Created by jakub on 24.10.2019.
//

#ifndef DEVCLIENT_UTILS_HH
#define DEVCLIENT_UTILS_HH

#include <fmt/format.h>
#include <glibmm.h>
#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

template <>
struct fmt::formatter<Glib::ustring>
{
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

template<class Elem, class Traits>
inline void hex_dump(const void *aData, std::size_t aLength,
    std::basic_ostream<Elem, Traits> &stream, std::size_t width = 16)
{
	const char* const start = static_cast<const char*>(aData);
	const char* const end = start + aLength;
	const char* line = start;

	while (line != end)
	{
		stream.width(4);
		stream.fill('0');
		stream << std::hex << line - start << " ";
		std::size_t lineLength = std::min(width, static_cast<std::size_t>(end - line));
		for (std::size_t pass = 1; pass <= 2; ++pass)
		{
			for (const char *next = line; next != end && next != line + width; ++next)
			{
				char ch = *next;
				switch (pass)
				{
					case 1:
						stream << (ch < 32 ? '.' : ch);
						break;
					case 2:
						if (next != line)
							stream << " ";

						stream.width(2);
						stream.fill('0');
						stream << std::hex <<
						    std::uppercase <<
						    static_cast<int>(static_cast<unsigned char>(ch));
						break;
				}
			}
			if (pass == 1 && lineLength != width)
				stream << std::string(width - lineLength, ' ');

			stream << " ";
		}

		stream << std::endl;
		line = line + lineLength;
	}
}

inline std::experimental::filesystem::path executable_dir()
{
	return (std::experimental::filesystem::read_symlink("/proc/self/exe").
	    parent_path().parent_path());
}

#endif //DEVCLIENT_UTILS_HH
