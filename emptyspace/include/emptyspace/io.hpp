#pragma once
#include <iterator>
#include <filesystem>
#include <sstream>
#include <fstream>

inline std::string ReadTextFile(const std::string_view filePath)
{
	if (!std::filesystem::exists(filePath.data()))
	{
		std::ostringstream message;
		message << "IO: File " << filePath.data() << " does not exist.";
		throw std::filesystem::filesystem_error(message.str(),
			std::make_error_code(std::errc::no_such_file_or_directory));
	}
	std::ifstream file(filePath.data());
	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}