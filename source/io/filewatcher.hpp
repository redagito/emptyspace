#pragma once

#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>

enum class FileStatus { Created, Modified, Erased };

class FileWatcher
{
public:
    std::string PathToWatch;
    std::chrono::duration<int, std::milli> Delay;

    FileWatcher(std::string pathToWatch, std::chrono::duration<int, std::milli> delay)
    : PathToWatch{pathToWatch}, Delay{delay}
{
        for (auto& file : std::filesystem::recursive_directory_iterator(PathToWatch))
        {
            _paths[file.path().string()] = std::filesystem::last_write_time(file);
        }
    }

    void Start(const std::function<void(std::string, FileStatus)>& action)
    {
        while (_running)
        {
            std::this_thread::sleep_for(Delay);

            auto it = _paths.begin();
            while (it != _paths.end()) 
            {
                if (!std::filesystem::exists(it->first)) 
                {
                    action(it->first, FileStatus::Erased);
                    it = _paths.erase(it);
                }
                else 
                {
                    ++it;
                }
            }

            for (auto& file : std::filesystem::recursive_directory_iterator(PathToWatch))
            {
                auto currentFileLastWriteTime = std::filesystem::last_write_time(file);

                if (!Contains(file.path().string())) 
                {
                    _paths[file.path().string()] = currentFileLastWriteTime;
                    action(file.path().string(), FileStatus::Created);
                }
                else
                {
                    if (_paths[file.path().string()] != currentFileLastWriteTime) 
                    {
                        _paths[file.path().string()] = currentFileLastWriteTime;
                        action(file.path().string(), FileStatus::Modified);
                    }
                }
            }
        }
    }
private:
    std::unordered_map<std::string, std::filesystem::file_time_type> _paths;
    bool _running = true;

    bool Contains(const std::string& key)
    {
        const auto iterator = _paths.find(key);
        return iterator != _paths.end();
    }
};