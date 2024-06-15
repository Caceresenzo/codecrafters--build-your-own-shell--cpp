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

void builtin_exit(const std::vector<std::string>& _)
{
	exit(0);
}

void builtin_echo(const std::vector<std::string>& arguments)
{
	size_t size = arguments.size();
	size_t last_index = size - 1;

	for (size_t index = 1; index < size; ++index)
	{
		std::cout << arguments[index];

		if (last_index != index)
			std::cout << " ";
	}

	std::cout << std::endl;
}

void builtin_type(const std::vector<std::string>& arguments)
{
	std::string program = arguments[1];

	builtin_map::iterator builtin = builtins.find(program);
	if (builtin != builtins.end())
	{
		std::cout << program << " is a shell builtin" << std::endl;
		return;
	}

	std::cout << program << ": not found" << std::endl;
}

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

int main()
{
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	builtins.insert(std::make_pair("exit", builtin_exit));
	builtins.insert(std::make_pair("echo", builtin_echo));
	builtins.insert(std::make_pair("type", builtin_type));

	std::string input;
	while (read(input))
	{
		eval(input);
	}
}
