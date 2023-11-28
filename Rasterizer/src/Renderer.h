#pragma once

#include <cstdint>
#include <vector>


#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;
		Vector4 NdcToScreen(Vector4 ndc) const;

		private:

		void WorldToScreen(std::vector<Mesh>& mesh) const;
		size_t AddMaterial(const std::string& path); 
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};
		std::vector<Material> m_Materials{};
		std::vector<Mesh> m_Meshes{};
	
		enum class RenderMode {
			standard, depth
		};
			
		RenderMode m_CurrentRenderMode{ RenderMode::standard};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};
		float m_Ar{};
	};
}