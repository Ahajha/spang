#include <cli151/cli151.hpp>
#include <cli151/macros.hpp>

#include <iostream>

struct CLI
{
	// TODO: cli151 should check that these are required
	const char* file = "";
	std::size_t min_freq;
};
CLI151_CLI(CLI, &T::file, &T::min_freq)

int main(int argc, char* argv[])
{
	const auto options = cli151::parse<CLI>(argc, argv);

	if (!options)
	{
		return 1;
	}

	const auto [file, min_freq] = *options;

	std::cout << "File: " << file << '\n';
	std::cout << "Min frequency: " << min_freq << '\n';
}
