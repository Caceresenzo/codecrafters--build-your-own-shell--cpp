#include "shell.hpp"

#include <iostream>
#include <unistd.h>
#include <climits>

bool locate(const std::string &program, std::string &output)
{
	const char *$path = getenv("PATH");
	if (!$path)
		return (false);

	std::vector<std::string> paths = split(std::string($path), ":");

	for (std::vector<std::string>::iterator iterator = paths.begin(); iterator != paths.end(); ++iterator)
	{
		std::string path = *iterator + "/" + program;

		if (access(path.c_str(), F_OK | X_OK) == 0)
		{
			output = path;
			return (true);
		}
	}

	return (false);
}

namespace builtins
{
	registry_map REGISTRY;

	void exit(const std::vector<std::string> &_)
	{
		std::exit(0);
	}

	void echo(const std::vector<std::string> &arguments)
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

	void type(const std::vector<std::string> &arguments)
	{
		std::string program = arguments[1];

		builtins::registry_map::iterator builtin = builtins::REGISTRY.find(program);
		if (builtin != builtins::REGISTRY.end())
		{
			std::cout << program << " is a shell builtin" << std::endl;
			return;
		}

		std::string path;
		if (locate(program, path))
		{
			std::cout << program << " is " << path << std::endl;
			return;
		}

		std::cout << program << ": not found" << std::endl;
	}

	void pwd(const std::vector<std::string> &_)
	{
		char path[PATH_MAX] = {};
		getcwd(path, sizeof(path));

		std::cout << path << std::endl;
	}

	void cd(const std::vector<std::string> &arguments)
	{
		std::string absolute_path;

		const std::string &path = arguments[1];
		if (path[0] == '/')
			absolute_path = path;
		else if (path[0] == '.')
		{
			char current_path[PATH_MAX] = {};
			getcwd(current_path, sizeof(current_path));

			absolute_path = std::string(current_path) + "/" + path;
		}
		else if (path[0] == '~')
		{
			const char *$home = getenv("HOME");
			if (!$home)
				std::cerr << "cd: $HOME is not set" << std::endl;
			else
				absolute_path = std::string($home) + "/" + path.substr(1 /* ~ */);
		}

		if (absolute_path.empty())
			return;

		if (chdir(absolute_path.c_str()) == -1)
			std::cout << "cd: " << path << ": No such file or directory" << std::endl;
	}

	void register_defaults()
	{
		REGISTRY.insert(std::make_pair("exit", exit));
		REGISTRY.insert(std::make_pair("echo", echo));
		REGISTRY.insert(std::make_pair("type", type));
		REGISTRY.insert(std::make_pair("pwd", pwd));
		REGISTRY.insert(std::make_pair("cd", cd));
	}
}