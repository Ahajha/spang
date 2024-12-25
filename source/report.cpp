#include <spang/report.hpp>

// temp
#include <iostream>

namespace spang
{

void report(const std::span<const dfs_edge_t> codes,
            const std::span<const dfs_projection_link> projections, const std::size_t codes_support)
{
	(void)projections;
	(void)codes_support;

	// Temporary: Need proper file opening and whatnot
	// Do the codes need any conversion?
	for (const auto& code : codes)
	{
		std::cout << '(' << code.from << ", " << code.to << ", " << code.from_label << ", "
				  << code.edge_label << ", " << code.to_label << ")\n";
	}
}

} // namespace spang
