#pragma once

#include <string>
#include <vector>
#include <stdint.h>
#include <leptonica/allheaders.h>

class Image
{
public:
	Image(const std::string& filename) : m_pixels(pixRead(filename.c_str())) {}
	Image(const std::vector<uint8_t> bytes) : m_pixels(pixReadMem(bytes.data(), bytes.size())) {}
	Image(const uint8_t * const bytes_ptr, size_t size) : m_pixels(pixReadMem(bytes_ptr, size)) {}

	virtual ~Image() noexcept { pixDestroy(&m_pixels); }

	Pix* pixels() const noexcept { return m_pixels; }

private:
	Pix* m_pixels;
};

