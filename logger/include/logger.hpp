#pragma once

#include <ctime>
#include <chrono>
#include <iostream>
#include <string_view>

namespace spang
{

void log_time(std::ostream& stream)
{
	char buf[16];
	std::time_t t = std::time(nullptr);
	std::strftime(buf, sizeof(buf), "[%H:%M:%S] ", std::localtime(&t));
	stream << buf;
}

template<class... Args>
void log_info(Args... args)
{
	log_time(std::clog);
	std::clog << "INFO: ";
	(std::clog << ... << std::forward<Args>(args)) << std::endl;
}

template<class... Args>
void log_error(Args... args)
{
	log_time(std::cerr);
	std::clog << "ERROR: ";
	(std::cerr << ... << std::forward<Args>(args)) << '\n';
	exit(1);
}

class timer
{
	const std::string_view msg;
	const decltype(std::chrono::high_resolution_clock::now()) start;
	
	public:
	
	timer(std::string_view m) : msg(m),
		start(std::chrono::high_resolution_clock::now()) {}
	
	~timer()
	{
		const auto end = std::chrono::high_resolution_clock::now();
		
		// std::chrono::hh_mm_ss would make this much simpler, but is
		// currently only available in gcc11, which at time of writing
		// is still very new, making installing a bit difficult.
		
		const auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>
			(end - start).count();
		
		// Seconds also encodes milliseconds.
		const auto s = (total_ms % (1000 * 60)) / 1000.0;
		const auto m = (total_ms / (1000 * 60)) % 60;
		const auto h = total_ms / (1000 * 60 * 60);
		
		// No need to print hours if the run took less than 1 hour, likewise for other units.
		if (h > 0)
			log_info(msg, h, "h ", m, "m ", s, 's');
		else if (m > 0)
			log_info(msg, m, "m ", s, 's');
		else
			log_info(msg, s, 's');
	}
	
	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;
	timer(timer&&) = delete;
	timer& operator=(timer&&) = delete;
};

}
