#include "shell.hpp"

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>

#define UP 'A'
#define DOWN 'B'

void prompt()
{
	std::cout << "$ " << std::flush;
}

void bell()
{
	std::cout << '\a' << std::flush;
}

void change_line(std::string &line, const std::string &new_line)
{
	size_t length = line.length();

	std::string backspaces(length, '\b');
	std::string spaces(length, ' ');

	std::cout << backspaces << spaces << backspaces << std::flush;

	std::cout << new_line << std::flush;

	line = new_line;
}

std::optional<int> exec(const parsing::ParsedLine &parsed_line)
{
	RedirectedStreams streams(parsed_line.redirects);
	if (!streams.valid())
		return (std::nullopt);

	const std::vector<std::string> &arguments = parsed_line.arguments;
	std::string program = arguments[0];

	builtins::registry_map::iterator builtin = builtins::REGISTRY.find(program);
	if (builtin != builtins::REGISTRY.end())
		return (builtin->second(arguments, streams));

	std::string path;
	if (!locate(program, path))
	{
		std::cout << program << ": command not found" << std::endl;
		return (std::nullopt);
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork");
		return (std::nullopt);
	}
	else if (pid == 0)
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

	return (std::nullopt);
}

std::optional<int> eval(std::string &line)
{
	history::add(line);

	auto commands = parsing::LineParser(line).parse();

	if (commands.size() == 1)
		return (exec(commands.front()));
	else
		pipeline(commands);

	return (std::nullopt);
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

	size_t history_length = history::get().size();
	size_t history_position = history_length;

	bool bell_rang = false;
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
			autocompletion::Result result = autocompletion::complete(line, bell_rang);

			switch (result)
			{
			case autocompletion::Result::NONE:
				bell();
				bell_rang = false;
				break;

			case autocompletion::Result::FOUND:
				bell_rang = false;
				break;

			case autocompletion::Result::MORE:
				bell();
				bell_rang = true;
				break;
			}
		}
		else if (character == 0x1b)
		{
			getchar(); // '['

			char direction = getchar();
			if (direction == UP && history_position != 0)
			{
				history_position--;
				change_line(line, history::get()[history_position]);
			}
			else if (direction == DOWN && history_position < history_length)
			{
				history_position++;

				if (history_position == history_length)
					change_line(line, "");
				else
					change_line(line, history::get()[history_position]);
			}
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

int loop()
{
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
			auto shell_exit_code = eval(input);
			if (shell_exit_code.has_value())
				return (shell_exit_code.value());
		}
	}
}

int main()
{
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	history::initialize();
	builtins::register_defaults();

	int exit_code = loop();

	history::finalize();

	return (exit_code);
}
