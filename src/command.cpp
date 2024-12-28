#include "shell.hpp"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <climits>
#include <unistd.h>
#include <fcntl.h>

bool locate(const std::string &program, std::string &output)
{
	if (program.starts_with('/'))
	{
		if (access(program.c_str(), F_OK | X_OK) == 0)
		{
			output = program;
			return (true);
		}

		return (false);
	}

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

RedirectedStreams::RedirectedStreams(const std::vector<Redirect> &redirects)
{
	std::optional<int> output;
	std::optional<int> error;

	for (auto redirect : redirects)
	{
		int flags = O_CREAT | O_WRONLY;
		if (redirect.append)
			flags |= O_APPEND;
		else
			flags |= O_TRUNC;

		int fd = open(redirect.path.c_str(), flags, 0644);
		if (fd == -1)
		{
			const char *error = strerror(errno);
			std::cerr << "shell: " << redirect.path << ": " << error;
			_valid = false;
			break;
		}

		if (StandardNamedStream::OUTPUT == redirect.stream_name)
		{
			if (output.has_value())
				::close(output.value());

			output = fd;
		}
		else if (StandardNamedStream::ERROR == redirect.stream_name)
		{
			if (error.has_value())
				::close(error.value());

			error = fd;
		}
		else
			::close(fd);
	}

	_output = output;
	_error = error;
	_valid = true;
}

RedirectedStreams::~RedirectedStreams()
{
	close();
}

void RedirectedStreams::close()
{
	if (_output.has_value())
	{
		::close(_output.value());
		_output.reset();
	}

	if (_error.has_value())
	{
		::close(_error.value());
		_error.reset();
	}
}

namespace builtins
{
	registry_map REGISTRY;

	void exit(const std::vector<std::string> &_, const RedirectedStreams &__)
	{
		std::exit(0);
	}

	void echo(const std::vector<std::string> &arguments, const RedirectedStreams &streams)
	{
		size_t size = arguments.size();
		size_t last_index = size - 1;

		for (size_t index = 1; index < size; ++index)
		{
			dprintf(streams.output(), "%s", arguments[index].c_str());

			if (last_index != index)
				dprintf(streams.output(), " ");
		}

		dprintf(streams.output(), "\n");
	}

	void type(const std::vector<std::string> &arguments, const RedirectedStreams &streams)
	{
		std::string program = arguments[1];

		builtins::registry_map::iterator builtin = builtins::REGISTRY.find(program);
		if (builtin != builtins::REGISTRY.end())
		{
			dprintf(streams.output(), "%s is a shell builtin\n", program.c_str());
			return;
		}

		std::string path;
		if (locate(program, path))
		{
			dprintf(streams.output(), "%s is %s\n", program.c_str(), path.c_str());
			return;
		}

		dprintf(streams.error(), "%s: not found\n", program.c_str());
	}

	void pwd(const std::vector<std::string> &_, const RedirectedStreams &streams)
	{
		char path[PATH_MAX] = {};
		getcwd(path, sizeof(path));

		dprintf(streams.output(), "%s\n", path);
	}

	void cd(const std::vector<std::string> &arguments, const RedirectedStreams &streams)
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
				dprintf(streams.error(), "cd: $HOME is not set\n");
			else
				absolute_path = std::string($home) + "/" + path.substr(1 /* ~ */);
		}

		if (absolute_path.empty())
			return;

		if (chdir(absolute_path.c_str()) == -1)
			dprintf(streams.output(), "cd: %s: %s\n", path.c_str(), strerror(errno));
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