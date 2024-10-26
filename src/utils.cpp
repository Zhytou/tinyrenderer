#include "utils.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

namespace tinyrenderer
{

std::string readText(const std::string& filename)
{
	std::ifstream file{filename};
	if(!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
	
std::vector<char> readBinary(const std::string& filename)
{
	std::ifstream file{filename, std::ios::binary | std::ios::ate};
	if(!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	file.read(buffer.data(), size);
	return buffer;
}
    
} // namespace tinyrenderer
