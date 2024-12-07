#include "shell.hpp"

#define END '\0'
#define SPACE ' '
#define SINGLE '\''
#define DOUBLE '"'
#define BACKSLASH '\\'

LineParser::LineParser(const std::string &line)
    : iterator(std::prev(line.begin())),
      end(line.end()),
      builder()
{
    builder.reserve(line.length());
}

std::vector<std::string> LineParser::parse(void)
{
    std::vector<std::string> strings;

    char character;
    while ((character = next()) != END)
    {
        switch (character)
        {
        case SPACE:
        {
            if (!builder.empty())
            {
                strings.push_back(builder);
                builder.clear();
            }

            break;
        }

        case BACKSLASH:
        {
            backslash(false);

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
                    backslash(true);
                else
                    builder.push_back(character);
            }

            break;
        }

        default:
        {
            builder.push_back(character);
            break;
        }
        }
    }

    if (!builder.empty())
        strings.push_back(builder);

    return (strings);
}

void LineParser::backslash(bool in_quote)
{
    char character = next();
    if (character == END)
        return;

    if (in_quote)
        builder += BACKSLASH;

    builder += character;
}

char LineParser::next(void)
{
    if (iterator != end)
        ++iterator;

    if (iterator == end)
        return (END);

    return (*iterator);
}
