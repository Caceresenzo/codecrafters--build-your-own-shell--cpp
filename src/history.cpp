#include "shell.hpp"

#include <fstream>

namespace history
{
    std::vector<std::string> lines;
    size_t last_append_index = 0;

    void add(const std::string &command)
    {
        lines.push_back(command);
    }

    const std::vector<std::string> &get()
    {
        return lines;
    }

    void read(const std::string &path)
    {
        std::ifstream file;

        file.open(path);
        if (!file.is_open())
            return;

        std::string line;
        while (std::getline(file, line))
        {
            if (!line.empty() && line[line.length() - 1] == '\n')
                line.erase(line.length() - 1);

            if (!line.empty() && line[line.length() - 1] == '\r')
                line.erase(line.length() - 1);

            if (!line.empty())
                lines.push_back(line);
        }

        file.close();
    }

    void write(const std::string &path)
    {
        std::ofstream file;

        file.open(path);
        if (!file.is_open())
            return;

        for (const auto &line : lines)
            file << line << std::endl;

        file.close();
    }

    void append(const std::string &path)
    {
        std::ofstream file;

        file.open(path, std::ios_base::app);
        if (!file.is_open())
            return;

        for (auto iterator = lines.begin() + last_append_index; iterator < lines.end(); ++iterator)
            file << *iterator << std::endl;
        
        last_append_index = lines.size();

        file.close();
    }
}
