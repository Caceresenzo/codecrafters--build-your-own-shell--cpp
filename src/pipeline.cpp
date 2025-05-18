#include <iostream>
#include <sys/wait.h>

#include "shell.hpp"

static int _exec(const parsing::ParsedLine &command)
{
    RedirectedStreams streams(command.redirects);
    if (!streams.valid())
        return (1);

    const std::vector<std::string> &arguments = command.arguments;
    const std::string &program = arguments[0];

    builtins::registry_map::iterator builtin = builtins::REGISTRY.find(program);
    if (builtin != builtins::REGISTRY.end())
    {
        builtin->second(arguments, streams);
        return (0);
    }

    std::string path;
    if (!locate(program, path))
    {
        std::cout << program << ": command not found" << std::endl;
        return (1);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return (1);
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

    // TODO Return waitpid's exit code
    return (0);
}

static pid_t spawn(int fd_in, int fd_out, const parsing::ParsedLine &command)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return (-1);
    }
    else if (pid == 0)
    {
        dup2(fd_in, STDIN_FILENO);
        dup2(fd_out, STDOUT_FILENO);

        exit(_exec(command));
    }

    return (pid);
}

pid_t pipeline(const std::list<parsing::ParsedLine> &commands)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return (-1);
    }
    else if (pid == 0)
    {
        int fd_in = STDIN_FILENO;

        auto iterator = commands.begin();
        for (size_t index = 0; index < commands.size() - 1; ++index)
        {
            const parsing::ParsedLine &command = *iterator++;

            int pipe_fds[2];
            pipe(pipe_fds);

            spawn(fd_in, pipe_fds[1], command);

            close(pipe_fds[1]);

            fd_in = pipe_fds[0];
        }

        dup2(fd_in, STDIN_FILENO);

        const parsing::ParsedLine &command = commands.back();
        exit(_exec(command));
    }

    int status = 0;
    waitpid(pid, &status, 0);

    return (pid);
}
