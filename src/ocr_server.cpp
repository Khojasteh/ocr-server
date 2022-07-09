#include <stdexcept>
#include "ocr_server.h"
#include "ocr_engine.h"

static const char* const RESPONSE_CONTENT_TYPE = "text/plain; charset=utf-8";
static const std::string AUTHENTICATION_SCHEME = "APIKEY";

enum HTTP_STATUS
{
	OK = 200,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	INTERNAL_SERVER_ERROR = 500,
};

static httplib::Server* create_server(const std::string& cert_path, const std::string& pkey_path)
{
	if (cert_path.empty() && pkey_path.empty())
		return new httplib::Server;

	if (cert_path.empty())
		throw std::invalid_argument("The SSL certificate file is not specified.");
	if (pkey_path.empty())
		throw std::invalid_argument("The SSL private key file is not specified.");

	return new httplib::SSLServer(cert_path.c_str(), pkey_path.c_str());
}

OcrServer::OcrServer(const OcrOptions& options) :
	m_options(options),
	m_server(create_server(options.ssl_certificate, options.ssl_private_key))
{
	if (!m_server || !m_server->is_valid())
		throw std::invalid_argument("The server configuration is not valid. Make sure hostname, port number, and SSL files are valid.");

	// POST /ocr/{language}
	m_server->Post("/ocr/((?:script/)?\\w+)", [this](const auto& req, auto& res) { process_image_to_text_request(req, res); });
	// GET /ocr/languages
	m_server->Get("/ocr/languages", [this](const auto& req, auto& res) { process_suppored_languages_request(req, res); });
}

bool OcrServer::validate_autorization(const httplib::Request& request, httplib::Response& response) noexcept
{
	if (!needs_authorization())
		return true;

	const auto autorization = request.get_header_value("Authorization");
	const auto i = autorization.find_first_of(' ');
	if (std::string::npos != i && autorization.substr(0, i) == AUTHENTICATION_SCHEME && is_authorized(autorization.substr(i + 1)))
		return true;

	response.status = HTTP_STATUS::UNAUTHORIZED;
	response.set_header("WWW-Authenticate", AUTHENTICATION_SCHEME);
	return false;
}


void OcrServer::process_suppored_languages_request(const httplib::Request& request, httplib::Response& response) noexcept
{
	if (!validate_autorization(request, response))
		return;

	response.status = HTTP_STATUS::OK;
	response.set_content(OcrEngine::get_supported_language_list(), RESPONSE_CONTENT_TYPE);
}

void OcrServer::process_image_to_text_request(const httplib::Request& request, httplib::Response& response) noexcept
{
	if (!validate_autorization(request, response))
		return;

	const auto language = request.matches[1].str();
	if (language.empty())
	{
		response.status = HTTP_STATUS::BAD_REQUEST;
		response.set_content("The language parameter is not specified.", RESPONSE_CONTENT_TYPE);
		return;
	}

	if (!OcrEngine::is_language_supported(language))
	{
		response.status = HTTP_STATUS::BAD_REQUEST;
		response.set_content("The specified language is not supported.", RESPONSE_CONTENT_TYPE);
		return;
	}

	const auto contentType = request.get_header_value("Content-Type");
	if (!OcrEngine::is_format_supported(contentType))
	{
		response.status = HTTP_STATUS::BAD_REQUEST;
		response.set_content("The content type is not supported.", RESPONSE_CONTENT_TYPE);
		return;
	}

	if (request.body.empty())
	{
		response.status = HTTP_STATUS::BAD_REQUEST;
		response.set_content("The request body is empty.", RESPONSE_CONTENT_TYPE);
		return;
	}

	const Image image{ reinterpret_cast<const uint8_t* const>(request.body.data()), request.body.size() };
	if (!image.pixels())
	{
		response.status = HTTP_STATUS::BAD_REQUEST;
		response.set_content("Invalid or unsupported image format.", RESPONSE_CONTENT_TYPE);
		return;
	}

	try
	{
		response.status = HTTP_STATUS::OK;
		response.set_content(OcrEngine::get_utf8_text(language, image), RESPONSE_CONTENT_TYPE);
	}
	catch (const std::exception& error)
	{
		response.status = HTTP_STATUS::INTERNAL_SERVER_ERROR;
		response.set_content(error.what(), RESPONSE_CONTENT_TYPE);
	}
}

