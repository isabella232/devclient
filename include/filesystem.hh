#ifndef DEVCLIENT_FILESYSTEM_HH
#define DEVCLIENT_FILESYSTEM_HH

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
#define GHC_USE_STD_FS
#include <filesystem>
namespace filesystem = std::filesystem;
#else
#include <ghc/filesystem.hpp>
namespace filesystem = ghc::filesystem;
#endif

#endif //DEVCLIENT_FILESYSTEM_HH
