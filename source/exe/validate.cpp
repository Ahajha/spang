#include "spang/logger.hpp"
#include "spang/parser.hpp"

#include <filesystem>
#include <fstream>

void read(spang::output_parser& parse, const std::filesystem::path& path)
{
	if (std::filesystem::is_directory(path))
	{
		for (const auto& sub_path : std::filesystem::directory_iterator(path))
		{
			if (!std::filesystem::is_directory(sub_path))
			{
				std::ifstream in(sub_path.path());
				parse.read(in);
			}
		}
	}
	else
	{
		std::ifstream in(path);
		parse.read(in);
	}
}

int main(int argc, char* argv[])
{
	spang::output_parser parse1, parse2;

	if (argc != 3)
		spang::log_error("usage: ", argv[0], " <path1> <path2>\n",
		                 "File paths read directly from the given file, ",
		                 "directories read all files immediately within them.");

	std::filesystem::path path1(argv[1]), path2(argv[2]);

	read(parse1, path1);
	read(parse2, path2);

	if (parse1.get_graphs() != parse2.get_graphs())
	{
		spang::log_info("Results differ");
	}
	else
	{
		spang::log_info("Results are the same");
	}
}
