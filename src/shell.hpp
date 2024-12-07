#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <map>
#include <functional>

std::vector<std::string> split(std::string haystack, const std::string &needle);
bool locate(const std::string &program, std::string &output);

namespace builtins
{
    using registry_map = std::map<std::string, std::function<void(const std::vector<std::string> &)>>;
    extern registry_map REGISTRY;

    void register_defaults();
}

class LineParser
{
private:
    std::string::const_iterator iterator;
    std::string::const_iterator end;
    std::string builder;

public:
    LineParser(const std::string &line);

public:
    std::vector<std::string> parse(void);

private:
    char next(void);
};

#endif
