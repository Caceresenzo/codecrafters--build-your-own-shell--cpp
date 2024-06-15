#include <iostream>
#include <string>

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

	std::string input;
	while (read(input))
	{
		std::cout << input << ": command not found" << std::endl;
	}
}
