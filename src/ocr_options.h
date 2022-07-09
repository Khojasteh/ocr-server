#pragma once

#include <string>
#include <unordered_set>

struct OcrOptions
{
	OcrOptions() noexcept :
		hostname("0.0.0.0"),
		port(8080),
		ssl_certificate(),
		ssl_private_key(),
		api_keys()
	{}

	OcrOptions(const std::string& filename)	: OcrOptions() { apply(filename);	}
	OcrOptions(int argc, const char* argv[]) : OcrOptions() { apply(argc, argv); }

	bool apply(int argc, const char* argv[]);
	bool apply(const std::string& filename);

	std::string hostname;
	int port;

	std::string ssl_certificate;
	std::string ssl_private_key;

	std::unordered_set<std::string> api_keys;
};

