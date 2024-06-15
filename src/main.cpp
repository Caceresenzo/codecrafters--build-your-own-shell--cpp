#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <unistd.h>
#include <sys/wait.h>
#include <climits>

bool read(std::string &input);
void eval(std::string &line);
void exec(const std::string &path, const std::vector<std::string> &arguments);

std::vector<std::string> split(std::string haystack, const std::string &needle)
{
	std::vector<std::string> result;

	while (!haystack.empty())
	{
		size_t index = haystack.find(needle);

		std::string argument = haystack.substr(0, index);
		result.push_back(argument);

		if (index != std::string::npos)
			index += needle.size();

		haystack.erase(0, index);
	}

	return result;
}

bool locate(const std::string &program, std::string &output)
{
	const char *$path = getenv("PATH");
	if (!$path)
		return (false);

	std::vector<std::string> paths = split(std::string($path), ":");

	for (std::vector<std::string>::iterator iterator = paths.begin(); iterator != paths.end(); ++iterator)
	{
		std::string path = *iterator + "/" + program;

		// std::cout << "?> " << path << std::endl;

		if (access(path.c_str(), F_OK | X_OK) == 0)
		{
			output = path;
			return (true);
		}
	}

	return (false);
}

using builtin_map = std::map<std::string, std::function<void(const std::vector<std::string> &)>>;
builtin_map builtins;

void builtin_exit(const std::vector<std::string> &_)
{
	exit(0);
}

void builtin_echo(const std::vector<std::string> &arguments)
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

void builtin_type(const std::vector<std::string> &arguments)
{
	std::string program = arguments[1];

	builtin_map::iterator builtin = builtins.find(program);
	if (builtin != builtins.end())
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

void builtin_pwd(const std::vector<std::string> &_)
{
	char path[PATH_MAX] = {};
	getcwd(path, sizeof(path));

	std::cout << path << std::endl;
}

void builtin_cd(const std::vector<std::string> &arguments)
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

	std::string path;
	if (locate(program, path))
	{
		exec(path, arguments);
		return;
	}

	std::cout << program << ": command not found" << std::endl;
}

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

int main()
{
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	builtins.insert(std::make_pair("exit", builtin_exit));
	builtins.insert(std::make_pair("echo", builtin_echo));
	builtins.insert(std::make_pair("type", builtin_type));
	builtins.insert(std::make_pair("pwd", builtin_pwd));
	builtins.insert(std::make_pair("cd", builtin_cd));

	std::string input;
	while (read(input))
	{
		eval(input);
	}
}
