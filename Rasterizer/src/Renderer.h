#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"

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

		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const;

	private:

		
		void NDCtoScreen(std::vector<Vertex>& vertices);
		float TriArea(const Vector3& p1, const Vector3& p2, const Vector3& p3);
		bool IsPointInTriangle(const Vector2& pos, const std::vector<Vertex>& triVertices);
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		//float* m_pDepthBufferPixels{};

		std::vector<Vertex> m_TriNDC{
			{{0.f,.5f,1.f}},
			{{.5f,-.5f,1.f}},
			{{-.5f,-.5f,1.f}}
		};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};
		int m_FOV{90};
		float m_Ar{};
	};
}
