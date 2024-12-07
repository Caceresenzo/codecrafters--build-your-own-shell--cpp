#include "shell.hpp"

#include <iostream>
#include <sys/wait.h>

void exec(const std::string &path, const std::vector<std::string> &arguments)
{
	pid_t pid = fork();

	if (pid == -1)
	{
		perror("fork");
		return;
	}

	if (pid == 0)
	{
		size_t size = arguments.size();

		char *argv[size + 1];
		argv[size] = NULL;

		for (size_t index = 0; index < size; ++index)
			argv[index] = const_cast<char *>(arguments[index].c_str());

		execvp(path.c_str(), argv);
		perror("execvp");
		exit(1);
	}
	else
		waitpid(pid, NULL, 0);
}

void eval(std::string &line)
{
	std::vector<std::string> arguments = split(line, " ");
	std::string program = arguments[0];

	builtins::registry_map::iterator builtin = builtins::REGISTRY.find(program);
	if (builtin != builtins::REGISTRY.end())
	{
		builtin->second(arguments);
		return;
	}

	std::string path;
	if (locate(program, path))
	{
		exec(path, arguments);
		return;
	}

	std::cout << program << ": command not found" << std::endl;
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

int main()
{
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	builtins::register_defaults();

	std::string input;
	while (read(input))
		eval(input);
}
