#include <iostream>
#include <fstream>
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include "ocr_options.h"

bool OcrOptions::apply(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.good())
		return false;

	nlohmann::json json;
	file >> json;

	hostname = json.value("hostname", hostname);
	port = json.value("port", port);
	ssl_certificate = json.value("ssl_certificate", ssl_certificate);
	ssl_private_key = json.value("ssl_private_key", ssl_private_key);
	for (const auto& key : json.value("api_keys", std::vector<std::string>()))
		api_keys.insert(key);

	return true;
}

bool OcrOptions::apply(int argc, const char* argv[])
{
	if (argc == 1)
		return false;

	std::vector<std::string> api_key_items;
	cxxopts::Options opts(argv[0], "ocrServer - A web based API for extracting text from images.\n");
	opts.add_options()
		(
			"help",
			"Displays this message and exits"
		)
		(
		"h, hostname",
		"The IP address or host name of the server to listen to",
		cxxopts::value<std::string>(hostname)->default_value(hostname)
		)
		(
			"p, port",
			"The IP address or port number of the server to listen to",
			cxxopts::value<int>(port)->default_value(std::to_string(port))
		)
		(
			"c, sslcert",
			"Path to the file containing the SSL certificate (PEM format)",
			cxxopts::value<std::string>(ssl_certificate)
		)
		(
			"k, sslkey",
			"Path to the file containing the SSL private key (PEM format)",
			cxxopts::value<std::string>(ssl_private_key)
		)
		(
			"a, apikey",
			"The authorized API key, multiple keys are allowed",
			cxxopts::value<std::vector<std::string>>(api_key_items)
		);

	auto result = opts.parse(argc, const_cast<char**&>(argv));
	if (result.count("help"))
	{
		std::cerr << opts.help() << std::endl;
		exit(1);
	}

	for (const auto& key : api_key_items)
		api_keys.insert(key);

	return true;
}
