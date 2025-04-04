// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#ifndef __MULTIOLED_HPP
#define __MULTIOLED_HPP

#include <MultiOled/version.h>
#include <filesystem>
#include <string>

// Public API

namespace dotname {

  class MultiOled {

    const std::string libName_ = std::string ("MultiOled v.") + MULTIOLED_VERSION;
    std::filesystem::path assetsPath_;

  public:
    MultiOled ();
    MultiOled (const std::filesystem::path& assetsPath);
    ~MultiOled ();

    const std::filesystem::path getAssetsPath () const {
      return assetsPath_;
    }
    void setAssetsPath (const std::filesystem::path& assetsPath) {
      assetsPath_ = assetsPath;
    }

    int selectI2CChannel (int channel);

    void oledTester ();

    std::string getCurrentTimeStr ();
    
  };

} // namespace dotname

#endif // __MULTIOLED_HPP