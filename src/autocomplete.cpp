#include "shell.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <functional>
#include <filesystem>

struct string_comparator
{
    bool operator()(const std::string &x, const std::string &y) const
    {
        return (std::less<size_t>()(x.length(), y.length()) || std::less<std::string>()(x, y));
    }
};

static void commit(std::string &line, const std::string &candidate)
{
    line += candidate;
    std::cout << candidate << std::flush;

    line += ' ';
    std::cout << ' ' << std::flush;
}

namespace autocompletion
{
    static void collect_builtins(std::set<std::string, string_comparator> &candidates, const std::string &line)
    {
        for (auto &builtin : builtins::REGISTRY)
        {
            const std::string &name = builtin.first;

            if (name.rfind(line, 0) == 0)
            {
                std::string candidate = name.substr(line.length());
                candidates.insert(candidate);
            }
        }
    }

    static void collect_executables(std::set<std::string, string_comparator> &candidates, const std::string &line)
    {
        const char *$path = getenv("PATH");
        if (!$path)
            return;

        std::vector<std::string> paths = split(std::string($path), ":");

        for (const auto &raw_path : paths)
        {
            std::filesystem::path path = raw_path;

            if (!std::filesystem::is_directory(path))
                continue;

            for (const auto &entry : std::filesystem::directory_iterator(path))
            {
                const std::string &name = entry.path().filename().string();
                if (name.rfind(line, 0) != 0)
                    continue;

                constexpr auto executable = std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec;
                if (!entry.is_regular_file() || (entry.status().permissions() & executable) == std::filesystem::perms::none)
                    continue;

                std::string candidate = name.substr(line.length());
                candidates.insert(candidate);
            }
        }
    }

    Result complete(std::string &line)
    {
        std::set<std::string, string_comparator> candidates;

        collect_builtins(candidates, line);
        collect_executables(candidates, line);

        if (candidates.empty())
            return (Result::NONE);
        else if (candidates.size() == 1)
        {
            commit(line, *candidates.begin());
            return (Result::FOUND);
        }

        return (Result::NONE);
    }
}
