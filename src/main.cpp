#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>

std::vector<std::string> split(std::string haystack, const std::string &needle)
{
	std::vector<std::string> result;

	while (!haystack.empty())
	{
		size_t index = haystack.find(needle);

		std::string argument = haystack.substr(0, index);
		result.push_back(argument);

		if (index != std::string::npos)
			++index; /* space */

		haystack.erase(0, index);
	}

	return result;
}

using builtin_map = std::map<std::string, std::function<void(const std::vector<std::string>&)>>;
builtin_map builtins;

bool read(std::string &input)
{
	while (true)
	{
		std::cout << "$ ";

		if (!std::getline(std::cin, input))
			return (false);

		if (!input.empty())
			return (true);
	}
}

void eval(std::string &line)
{
	std::vector<std::string> arguments = split(line, " ");
	std::string program = arguments[0];

	builtin_map::iterator builtin = builtins.find(program);
	if (builtin != builtins.end())
	{
		builtin->second(arguments);
		return;
	}

	std::cout << program << ": command not found" << std::endl;
}

void builtin_exit(const std::vector<std::string>& _)
{
	exit(0);
}

int main()
{
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	builtins.insert(std::make_pair("exit", builtin_exit));

	std::string input;
	while (read(input))
	{
		eval(input);
	}
}
