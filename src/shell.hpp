#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <optional>
#include <list>
#include <unistd.h>

std::vector<std::string> split(std::string haystack, const std::string &needle);
bool locate(const std::string &program, std::string &output);

enum class StandardNamedStream
{
    UNKNOWN = -1,
    OUTPUT = 1,
    ERROR = 2
};

typedef struct
{
    StandardNamedStream stream_name;
    std::string path;
    bool append;
} Redirect;

class RedirectedStreams
{
private:
    bool _valid;
    std::optional<int> _output;
    std::optional<int> _error;

public:
    RedirectedStreams(const std::vector<Redirect> &redirects);
    ~RedirectedStreams();

public:
    void close();

    inline bool valid() const
    {
        return (_valid);
    }

    inline int output() const
    {
        return (_output.value_or(STDOUT_FILENO));
    }

    inline int error() const
    {
        return (_error.value_or(STDERR_FILENO));
    }
};

namespace builtins
{
    using registry_map = std::map<std::string, std::function<void(const std::vector<std::string> &, const RedirectedStreams &)>>;
    extern registry_map REGISTRY;

    void register_defaults();
}

namespace parsing
{
    typedef struct
    {
        std::vector<std::string> arguments;
        std::vector<Redirect> redirects;
    } ParsedLine;

    class LineParser
    {
    private:
        std::string::const_iterator iterator;
        std::string::const_iterator end;
        std::list<parsing::ParsedLine> commands;
        std::vector<std::string> arguments;
        std::vector<Redirect> redirects;

    public:
        LineParser(const std::string &line);

    public:
        std::list<parsing::ParsedLine> parse(void);

    private:
        std::optional<std::string> next_argument();
        void backslash(std::string &builder, bool in_quote);
        char map_backslash_character(char character);
        void redirect(StandardNamedStream stream_name);
        void pipe(void);
        char next(void);
        char peek(void);
        StandardNamedStream get_steam_name_from_fd(char character);
    };
}

namespace autocompletion
{
    enum class Result
    {
        NONE,
        FOUND,
        MORE,
    };

    Result complete(std::string &line, bool bell_rang);
}

void prompt();
pid_t pipeline(const std::list<parsing::ParsedLine> &commands);

namespace history
{
    void add(const std::string &command);
    const std::vector<std::string> &get();
    void read(const std::string &path);
    void write(const std::string &path);
    void append(const std::string &path);
}

#endif
