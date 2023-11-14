//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	m_Ar = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	
	//RENDER LOGIC
	std::vector<Vertex> triScreenPoints{};
	VertexTransformationFunction(m_TriNDC, triScreenPoints);

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			//float gradient = px / static_cast<float>(m_Width);
			//gradient += py / static_cast<float>(m_Width);
			//gradient /= 2.0f;
			//
			ColorRGB finalColor{ };
			if (IsPointInTriangle({ px,py }, triScreenPoints)) {
				finalColor = {1,1,1};
			}
			
			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage
	for (size_t i = 0; i < vertices_in.size(); i++)
	{
		float projectedX = vertices_in[i].position.x / vertices_in[i].position.z;
		projectedX /= m_Ar * m_FOV;
		projectedX = (projectedX + 1) / 2 * m_Width;
		float projectedY = vertices_in[i].position.y / vertices_in[i].position.z;
		projectedY /= m_FOV;
		projectedY = (projectedY + 1) / 2 * m_Width;
		float projectedZ = vertices_in[i].position.z;
		vertices_out.push_back(Vertex({ projectedX,projectedY,projectedZ }));

	}
}


bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

float Renderer::TriArea(const Vector3& p1, const Vector3& p2, const Vector3& p3)
{
	return abs((p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y)) / 2.0f);
}

bool dae::Renderer::IsPointInTriangle(const Vector2& pos, const std::vector<Vertex>& triVertices)
{
	Vector3 position{ pos.x,pos.y,0 };
	const float A{ TriArea(triVertices[0].position, triVertices[1].position, triVertices[2].position) };
	const float A1{ TriArea(position, triVertices[1].position, triVertices[2].position) };
	const float A2{ TriArea(triVertices[0].position, position, triVertices[2].position) };
	const float A3{ TriArea(triVertices[0].position, triVertices[1].position, position) };
	return { A == (A1 + A2 + A3) };
	
}
