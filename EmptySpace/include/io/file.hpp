#pragma once

#include <iterator>
#include <filesystem>
#include <fstream>

inline std::string ReadTextFile(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath);

    if (!file)
    {
        throw std::runtime_error("Failed to open " + filePath.string());
    }

    std::string data = { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

    if (file.fail())
    {
        throw std::runtime_error("Error reading " + filePath.string());
    }

    return data;
}