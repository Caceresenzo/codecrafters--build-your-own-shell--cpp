#include "shell.hpp"

std::vector<std::string> split(std::string haystack, const std::string &needle)
{
    std::vector<std::string> result;

    while (!haystack.empty())
    {
        size_t index = haystack.find(needle);

        std::string argument = haystack.substr(0, index);
        result.push_back(argument);

        if (index != std::string::npos)
            index += needle.size();

        haystack.erase(0, index);
    }

    return result;
}
