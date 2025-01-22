#include "shell.hpp"

#include <iostream>
#include <set>
#include <functional>

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
    std::cout << candidate << ' ' << std::flush;
}

namespace autocompletion
{
    Result complete(std::string &line)
    {
        std::set<std::string, string_comparator> candidates;

        for (auto &builtin : builtins::REGISTRY)
        {
            const std::string &name = builtin.first;

            if (name.rfind(line, 0) == 0)
            {
                std::string candidate = name.substr(line.length());
                candidates.insert(candidate);
            }
        }

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
