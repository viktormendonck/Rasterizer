//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"
#define triangleStrip


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

	m_pDepthBufferPixels = new float[m_Width * m_Height];
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
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

	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	m_Ar = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	std::vector<Mesh> worldMeshes{
		Mesh{{
			Vertex{{-3.f, 3.f,-2.f}},
			Vertex{{ 0.f, 3.f,-2.f}},
			Vertex{{ 3.f, 3.f,-2.f}},
			Vertex{{-3.f, 0.f,-2.f}},
			Vertex{{ 0.f, 0.f,-2.f}},
			Vertex{{ 3.f, 0.f,-2.f}},
			Vertex{{-3.f,-3.f,-2.f}},
			Vertex{{ 0.f,-3.f,-2.f}},
			Vertex{{ 3.f,-3.f,-2.f}}},
			//{3,0,4,1,5,2,2,6,6,3,7,4,8,5}, // triangleStripIndices
			//PrimitiveTopology::TriangleStrip
			{
			 3,0,1,  1,4,3,  4,1,2,
			 2,5,4,  6,3,4,  4,7,6,
			 7,4,5,  5,8,7
			},
			PrimitiveTopology::TriangleList
		}
	};

	std::vector<Mesh> screenMeshes{};
	WorldToView(worldMeshes, screenMeshes);
	for (const Mesh& worldMesh : screenMeshes)
	{
		//RENDER LOGIC

		const size_t screenPixels{ static_cast<size_t>(m_Width * m_Height) };
	
		SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
		//logic for topology
		int itterationAmnt{};
		size_t amntOfChecks{};
		if (worldMesh.primitiveTopology == PrimitiveTopology::TriangleList) 
		{
			itterationAmnt = 3;
			amntOfChecks = worldMesh.indices.size();
		}
		else 
		{
			itterationAmnt = 1;
			amntOfChecks = worldMesh.indices.size() - 2;
		}

		for (size_t triIdx = 0; triIdx < amntOfChecks; triIdx += itterationAmnt)
		{
			int idx0{ static_cast<int>(triIdx) + 0 };
			int idx1{ static_cast<int>(triIdx) + 1 };
			int idx2{ static_cast<int>(triIdx) + 2 };

			if (worldMesh.primitiveTopology == PrimitiveTopology::TriangleStrip) {
				if (triIdx % 2 != 0) {
					std::swap(idx0, idx2);
				}
			}

			const Vertex& v0 = worldMesh.vertices[worldMesh.indices[idx0]];
			const Vertex& v1 = worldMesh.vertices[worldMesh.indices[idx1]];
			const Vertex& v2 = worldMesh.vertices[worldMesh.indices[idx2]];

			//check if tri is behind you
			if (v0.position.z <= 0 ||
				v1.position.z <= 0 ||
				v2.position.z <= 0)
			{
				continue;
			}


			//get bounding boxes
			Vector2i topLeft{
				static_cast<int>(std::min(std::min(v0.position.x, v1.position.x), v2.position.x)),
				static_cast<int>(std::min(std::min(v0.position.y, v1.position.y), v2.position.y))
			};
			Vector2i bottomRight{
				static_cast<int>(std::max(std::max(v0.position.x, v1.position.x), v2.position.x)),
				static_cast<int>(std::max(std::max(v0.position.y, v1.position.y), v2.position.y))
			};
			topLeft.x = std::max(topLeft.x, 0);
			topLeft.x = std::min(topLeft.x, m_Width);
			topLeft.y = std::max(topLeft.y, 0);
			topLeft.y = std::min(topLeft.y, m_Height);

			bottomRight.x = std::max(bottomRight.x, 0);
			bottomRight.x = std::min(bottomRight.x, m_Width);
			bottomRight.y = std::max(bottomRight.y, 0);
			bottomRight.y = std::min(bottomRight.y, m_Height);


			for (int px{ topLeft.x }; px < bottomRight.x; ++px)
			{
				for (int py{ topLeft.y }; py < bottomRight.y; ++py)
				{

					Vector2i pos{ px,py };
					ColorRGB finalColor{ };
					Vector3 weights{};

					if (dae::Utils::IsInTriangle(pos.toVector2(), v0.position, v1.position, v2.position, weights))
					{
						int pixelIdx{ px + py * m_Width };
						const float hitDepth{
							v0.position.z * weights.x +
							v1.position.z * weights.y +
							v2.position.z * weights.z
						};

						if (m_pDepthBufferPixels[pixelIdx] < hitDepth) { continue; }
						else { m_pDepthBufferPixels[pixelIdx] = hitDepth; }

						finalColor = {
							v0.color * weights.x +
							v1.color * weights.y +
							v2.color * weights.z
						};

						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}
			}
		}

	}

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

Vector3 dae::Renderer::NdcToScreen(Vector3 ndc) const
{
	ndc.x = (ndc.x + 1) / 2 * m_Width;
	ndc.y = (1 - ndc.y) / 2 * m_Height;
	return ndc;
}
bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}



void dae::Renderer::WorldToView(const std::vector<Mesh>& inMesh, std::vector<Mesh>& outMesh) const
{
	const float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	outMesh.reserve(inMesh.size());
	for (size_t meshIdx{}; meshIdx < inMesh.size(); meshIdx++)
	{
		std::vector<Vertex> temp{};
		for (size_t vertexIdx{}; vertexIdx < inMesh[meshIdx].vertices.size(); vertexIdx++)
		{
			const Vector3 view{ m_Camera.viewMatrix.TransformPoint(inMesh[meshIdx].vertices[vertexIdx].position)};

			float projectedX{ view.x / view.z };
			float projectedY{ view.y / view.z };
			projectedX /= (aspectRatio * m_Camera.fov);
			projectedY /= m_Camera.fov;

			const Vector3 projected{ projectedX, projectedY, view.z };
			temp.push_back(Vertex(NdcToScreen(projected), inMesh[meshIdx].vertices[vertexIdx].color));
		}
		outMesh.emplace_back(Mesh{ temp,inMesh[meshIdx].indices,inMesh[meshIdx].primitiveTopology});
	}
}




