#include "shell.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <functional>
#include <filesystem>

namespace autocompletion
{
    struct string_comparator
    {
        bool operator()(const std::string &x, const std::string &y) const
        {
            return (std::less<size_t>()(x.length(), y.length()) || std::less<std::string>()(x, y));
        }
    };

    static void commit(std::string &line, const std::string &candidate, bool has_more)
    {
        line += candidate;
        std::cout << candidate << std::flush;

        if (has_more)
            return;

        line += ' ';
        std::cout << ' ' << std::flush;
    }

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

    static std::optional<std::string> find_shared_prefix(const std::set<std::string, string_comparator> &candidates)
    {
        std::string first = *candidates.begin();
        if (first.empty())
            return (std::nullopt);

        std::vector<std::string> others(std::next(candidates.begin()), candidates.end());

        size_t end = 0;
        for (; end <= first.size(); ++end)
        {
            bool one_is_not_matching = false;

            for (const auto &other : others)
            {
                std::string_view other_truncated = other.substr(0, end);

                if (first.rfind(other_truncated, 0) != 0)
                {
                    one_is_not_matching = true;
                    break;
                }
            }

            if (one_is_not_matching)
            {
                --end;
                break;
            }
        }

        if (end == 0)
            return (std::nullopt);

        return (first.substr(0, end));
    }

    Result complete(std::string &line, bool bell_rang)
    {
        std::set<std::string, string_comparator> candidates;

        collect_builtins(candidates, line);
        collect_executables(candidates, line);

        if (candidates.empty())
            return (Result::NONE);
        else if (candidates.size() == 1)
        {
            commit(line, *candidates.begin(), false);
            return (Result::FOUND);
        }

        auto shared_prefix = find_shared_prefix(candidates);
        if (shared_prefix.has_value())
        {
            commit(line, shared_prefix.value(), true);
            return (Result::FOUND);
        }

        if (bell_rang)
        {
            std::cout << std::endl;

            size_t index = 0;
            for (const auto &candidate : candidates)
            {
                if (index++ != 0)
                    std::cout << "  ";

                std::cout << line;
                std::cout << candidate;
            }

            std::cout << std::endl;

            prompt();
            std::cout << line << std::flush;
        }

        return (Result::MORE);
    }
}
