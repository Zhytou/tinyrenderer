#pragma once

#include <vector>
#include <string>

namespace tinyrenderer
{
std::string readText(const std::string& filename);

std::vector<char> readBinary(const std::string& filename);
} // namespace tinyrenderer
