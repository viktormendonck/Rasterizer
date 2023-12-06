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

		void CycleRenderMode();
		void CycleCullingMode();
		void CycleShadingMode();
		void ToggleSpin();
		void ToggleNormalMap();

		ColorRGB  PixelShading(const Vertex_Out& v, int matId);

		private:
			

		
		void WorldToScreen(std::vector<Mesh>& mesh) const;
		size_t AddMaterial(const std::string& diffuse, const std::string& normal, const std::string& specular, const std::string& gloss);
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};
		std::vector<Material> m_Materials{};
		std::vector<Mesh> m_Meshes{};

	
		enum class RenderMode {
			standard, 
			depth, 
		};
		enum class CullingMode {
			partial,
			complete,
			off
		};
		enum class ShadingMode {
			Combined,
			ObservedArea,
			Diffuse,
			Specular	
		};


		bool m_isRotating{true};
		bool m_UsingNormalMap{true};

		CullingMode m_CurrentCullingMode{ CullingMode::complete };
			
		RenderMode m_CurrentRenderMode{ RenderMode::depth };
		ShadingMode m_CurrentShadingMode{ ShadingMode::Combined };

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};
		float m_Ar{};
	};
}