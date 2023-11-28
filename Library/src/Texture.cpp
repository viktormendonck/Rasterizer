#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include "Utils.h"
namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)
		SDL_Surface* image = IMG_Load(path.c_str());
		return new Texture(image);
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		int x{ static_cast<int>(uv.x * m_pSurface->w) };
		int y{ static_cast<int>(uv.y * m_pSurface->h) };

		const uint32_t* pPixels{ static_cast<uint32_t*>(m_pSurface->pixels) };
		const int pixelIdx{ static_cast<int>(x) + static_cast<int>(y) * m_pSurface->w };
		const uint32_t pixel{ pPixels[pixelIdx] };

		uint8_t r{};
		uint8_t g{};
		uint8_t b{};

		SDL_GetRGB(pixel, m_pSurface->format, &r,&g,&b);
			

		return {
			static_cast<float>(r)/255.f,
			static_cast<float>(g)/255.f,
			static_cast<float>(b)/255.f
		};
	}
}