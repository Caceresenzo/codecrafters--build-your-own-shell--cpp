#include "shell.hpp"

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>

void prompt()
{
	std::cout << "$ " << std::flush;
}

void exec(const std::string &path, const std::vector<std::string> &arguments, RedirectedStreams &streams)
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

		dup2(streams.output(), STDOUT_FILENO);
		dup2(streams.error(), STDERR_FILENO);

		streams.close();

		execvp(path.c_str(), argv);
		perror("execvp");
		exit(1);
	}
	else
		waitpid(pid, NULL, 0);
}

void eval(std::string &line)
{
	auto parsed_line = parsing::LineParser(line).parse();

	RedirectedStreams streams(parsed_line.redirects);
	if (!streams.valid())
		return;

	std::vector<std::string> &arguments = parsed_line.arguments;
	std::string program = arguments[0];

	builtins::registry_map::iterator builtin = builtins::REGISTRY.find(program);
	if (builtin != builtins::REGISTRY.end())
	{
		builtin->second(arguments, streams);
		return;
	}

	std::string path;
	if (locate(program, path))
	{
		exec(path, arguments, streams);
		return;
	}

	std::cout << program << ": command not found" << std::endl;
}

struct termios_prompt
{

	struct termios previous;

	termios_prompt()
	{
		tcgetattr(STDIN_FILENO, &previous);

		struct termios new_ = previous;
		new_.c_lflag &= ~(ECHO | ICANON);
		new_.c_cc[VMIN] = 1;
		new_.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSANOW, &new_);
	}

	~termios_prompt()
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &previous);
	}
};

enum class ReadResult
{
	QUIT,
	EMPTY,
	CONTENT,
};

ReadResult read(std::string &line)
{
	line.clear();

	prompt();

	termios_prompt _;

	while (true)
	{
		char character = getchar();

		if (character == 0x4)
		{
			if (line.empty())
				return (ReadResult::QUIT);
		}
		else if (character == '\n')
		{
			std::cout << std::endl;
			return (line.empty() ? ReadResult::EMPTY : ReadResult::CONTENT);
		}
		else if (character == '\t')
		{
			autocompletion::Result result = autocompletion::complete(line);

			if (result == autocompletion::Result::FOUND)
				continue;
		}
		else if (character == 0x1b)
		{
			getchar(); // '['
			getchar(); // 'A' or 'B' or 'C' or 'D'
		}
		else if (character == 0x7f)
		{
			if (line.empty())
				continue;

			std::cout << "\b \b" << std::flush;
			line.pop_back();
		}
		else
		{
			std::cout << character << std::flush;
			line.push_back(character);
		}
	}
}

int main()
{
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	builtins::register_defaults();

	std::string input;
	while (true)
	{
		switch (read(input))
		{
		case ReadResult::QUIT:
			return (0);
		case ReadResult::EMPTY:
			continue;
		case ReadResult::CONTENT:
			eval(input);
		}
	}
}
