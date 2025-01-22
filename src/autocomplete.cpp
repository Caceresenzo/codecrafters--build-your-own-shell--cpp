#include "shell.hpp"

#include <iostream>
#include <set>
#include <functional>
#include <unistd.h>
#include <cstring>
#include <climits>
#include <unistd.h>
#include <fcntl.h>

// template struct string_comparator : std::binary_function<std::string, std::string, bool>
// {
// 	bool operator()(const T &x, const T &y) const
// 	{
// 		return (std::less<size_t>()(x.length(), y.length()) || std::less<std::string>()(x, y));
// 	}
// };

// static void commit(const std::string &line)
// {
// 	std::cout << line << std::endl;
// }

// namespace autocompletion
// {
// 	Result complete(std::string &line)
// 	{
// 		std::set<std::string, string_comparator> candidates;

// 		for (auto &builtin : builtins::REGISTRY)
// 		{
// 			const std::string &name = builtin.first;

// 			if (name.rfind(line, 0) == 0))
// 			{
// 				std::string candidate = name.substr(line.length());
// 				candidates.insert(candidate);
// 			}
// 		}

// 		if (candidates.empty())
// 			return (Result::NONE);
// 		else if (candidates.size() == 1)
// 		{
// 			line += *candidates.begin();
// 			return (Result::FOUND);
// 		}

// 		return (Result::NONE);
// 	}
// }