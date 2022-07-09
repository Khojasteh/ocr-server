#include <stdexcept>
#include <iostream>
#include "ocr_options.h"
#include "ocr_server.h"

int main(int argc, const char* argv[])
{
	try
	{
		OcrOptions options("ocrServer.config.json");
		options.apply(argc, argv);

		OcrServer server(options);
		std::cout << "Listening on " << server.base_url() << std::endl;
		std::cout << "Press Ctrl+C to shut down." << std::endl;
		server.listen();
		return 0;
	}
	catch (const std::exception & error)
	{
		std::cerr << error.what();
		return 1;
	}
}
