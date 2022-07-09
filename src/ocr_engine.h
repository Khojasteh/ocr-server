#pragma once

#include <string>
#include <unordered_set>
#include "image.h"

class OcrEngine
{
public:
	static bool is_format_supported(const std::string& mime_type) noexcept { return supported_formats.find(mime_type) != supported_formats.end(); }
	static bool is_language_supported(const std::string& language) noexcept { return supported_languages.find(language) != supported_languages.end(); }
	static std::string get_supported_language_list() noexcept { return supported_language_list; }
	static std::string get_utf8_text(const std::string& language, const Image& image);

private:
	static const std::unordered_set<std::string> supported_formats;
	static const std::unordered_set<std::string> supported_languages;
	static const std::string supported_language_list;
};

