#pragma once

#include <string>
#include <memory>
#include <httplib.h>
#include "ocr_options.h"

class OcrServer
{
public:
	OcrServer(const OcrOptions& options);
	
	bool is_secure() const noexcept { return dynamic_cast<httplib::SSLServer*>(m_server.get()) != nullptr; }
	std::string base_url() { return (is_secure() ? std::string("https://") : std::string("http://")) + m_options.hostname + ':' + std::to_string(m_options.port); }

	bool needs_authorization() const noexcept { return !m_options.api_keys.empty(); }
	bool is_authorized(const std::string& api_key) const noexcept { return m_options.api_keys.find(api_key) != m_options.api_keys.end(); }

	bool listen() noexcept { return m_server->listen(m_options.hostname.c_str(), m_options.port); }

private:
	bool validate_autorization(const httplib::Request& request, httplib::Response& response) noexcept;
	void process_suppored_languages_request(const httplib::Request& request, httplib::Response& response) noexcept;
	void process_image_to_text_request(const httplib::Request& request, httplib::Response& response) noexcept;

private:
	const OcrOptions m_options;
	const std::unique_ptr<httplib::Server> m_server;
};
