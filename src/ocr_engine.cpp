#include "ocr_engine.h"
#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>
#include <algorithm>
#include <stdexcept>

const std::unordered_set<std::string> OcrEngine::supported_formats
{
	"image/png",
	"image/gif",
	"image/bmp",
	"image/jpeg",
	"image/tiff",
	"image/webp",
};

const std::unordered_set<std::string> OcrEngine::supported_languages = []
{
	std::unordered_set<std::string> languages;

	tesseract::TessBaseAPI api;
	if (api.Init(nullptr, nullptr) == 0)
	{
		GenericVector<STRING> langs;
		api.GetAvailableLanguagesAsVector(&langs);
		for (int i = 0, count = langs.size(); i < count; ++i)
			languages.insert(langs[i].c_str());
	}

	return languages;
}();

const std::string OcrEngine::supported_language_list = []
{
	std::vector<std::string> list{ supported_languages.begin(), supported_languages.end() };
	std::sort(list.begin(), list.end());

	std::string languages;
	languages.reserve(list.size() * 10);
	for (auto language : list)
	{
		if (languages.size() != 0)
			languages.append(1, '\n');
		languages.append(language);
	}
	languages.shrink_to_fit();
	return languages;
}();

std::string OcrEngine::get_utf8_text(const std::string& language, const Image& image)
{
	tesseract::TessBaseAPI api;
	if (api.Init(nullptr, language.c_str()) != 0)
		throw std::runtime_error("Could not initialize Tesseract.");
	
	api.SetDebugVariable("debug_file", "/dev/null");
	api.SetImage(image.pixels());
	auto text = api.GetUTF8Text();
	api.End();

	return std::move(text);
}
