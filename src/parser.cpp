#include "shell.hpp"

#define END '\0'
#define SPACE ' '
#define SINGLE '\''
#define DOUBLE '"'
#define BACKSLASH '\\'
#define GREATER_THAN '>'
#define PIPE '|'

namespace parsing
{
    LineParser::LineParser(const std::string &line)
        : iterator(std::prev(line.begin())),
          end(line.end()),
          commands(),
          arguments(),
          redirects()
    {
    }

    std::list<parsing::ParsedLine> LineParser::parse(void)
    {
        std::optional<std::string> argument;
        while (argument = next_argument())
            arguments.push_back(argument.value());
        
        if (!arguments.empty())
            pipe();

        return (commands);
    }

    std::optional<std::string> LineParser::next_argument()
    {
        std::string builder;

        char character;
        while ((character = next()) != END)
        {
            switch (character)
            {
            case SPACE:
            {
                if (!builder.empty())
                    return (std::optional(builder));

                break;
            }

            case BACKSLASH:
            {
                backslash(builder, false);

                break;
            }

            case SINGLE:
            {
                while ((character = next()) != END && character != SINGLE)
                    builder.push_back(character);

                break;
            }

            case DOUBLE:
            {
                while ((character = next()) != END && character != DOUBLE)
                {
                    if (character == BACKSLASH)
                        backslash(builder, true);
                    else
                        builder.push_back(character);
                }

                break;
            }

            case GREATER_THAN:
            {
                redirect(StandardNamedStream::OUTPUT);

                break;
            }

            case PIPE:
            {
                pipe();

                break;
            }

            default:
            {
                if (std::isdigit(character) && peek() == GREATER_THAN)
                {
                    next();
                    redirect(get_steam_name_from_fd(character));
                }
                else
                    builder.push_back(character);

                break;
            }
            }
        }

        if (!builder.empty())
            return (std::optional(builder));

        return (std::optional<std::string>());
    }

    void LineParser::backslash(std::string &builder, bool in_quote)
    {
        char character = next();
        if (character == END)
            return;

        if (in_quote)
        {
            char mapped = map_backslash_character(character);

            if (mapped != END)
                character = mapped;
            else
                builder += BACKSLASH;
        }

        builder += character;
    }

    char LineParser::map_backslash_character(char character)
    {
        if (character == BACKSLASH || character == DOUBLE)
            return (character);

        return (END);
    }

    void LineParser::redirect(StandardNamedStream stream_name)
    {
        bool append = peek() == GREATER_THAN;
        if (append)
            next();

        std::string path = next_argument().value();

        redirects.push_back(Redirect{
            .stream_name = stream_name,
            .path = path,
            .append = append});
    }

    void LineParser::pipe(void)
    {
        commands.push_back(ParsedLine{
            .arguments = arguments,
            .redirects = redirects
        });

        arguments.clear();
        redirects.clear();
    }

    char LineParser::next(void)
    {
        if (iterator != end)
            ++iterator;

        if (iterator == end)
            return (END);

        return (*iterator);
    }

    char LineParser::peek(void)
    {
        std::string::const_iterator next = iterator;
        if (iterator != end)
            next = std::next(iterator);

        if (next == end)
            return (END);

        return (*next);
    }

    StandardNamedStream LineParser::get_steam_name_from_fd(char character)
    {
        if (character == '1')
            return (StandardNamedStream::OUTPUT);
        else if (character == '2')
            return (StandardNamedStream::ERROR);
        else
            return (StandardNamedStream::UNKNOWN);
    }
}