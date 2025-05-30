#include "shell.hpp"

namespace history {

    std::vector<std::string> lines;

    void add(const std::string &command) {
        lines.push_back(command);
    }

    const std::vector<std::string>& get() {
        return lines;
    }

}