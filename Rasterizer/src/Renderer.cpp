//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"
#include <iostream>
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


	
	//m_Meshes.push_back(
	//	Mesh{
	//		{},{},PrimitiveTopology::TriangleList,AddMaterial("Resources/tuktuk.png")
	//	}
	//);
	//
	//Utils::ParseOBJ("Resources/tuktuk.obj", m_Meshes[0].vertices,m_Meshes[0].indices);

	m_Meshes.push_back(
		Mesh{
		{
			Vertex{{-3.f, 3.f,-2.f},{0.f,0.f}},
			Vertex{{ 0.f, 3.f,-2.f},{.5f,0.f}},
			Vertex{{ 3.f, 3.f,-2.f},{1.f,0.f}},
			Vertex{{-3.f, 0.f,-2.f},{0.f,.5f}},
			Vertex{{ 0.f, 0.f,-2.f},{.5f,.5f}},
			Vertex{{ 3.f, 0.f,-2.f},{1.f,.5f}},
			Vertex{{-3.f,-3.f,-2.f},{0.f,1.f}},
			Vertex{{ 0.f,-3.f,-2.f},{.5f,1.f}},
			Vertex{{ 3.f,-3.f,-2.f},{1.f,1.f}}
		},
	
		//{3,0,4,1,5,2,2,6,6,3,7,4,8,5}, 
		//PrimitiveTopology::TriangleStrip
	
		{
			3,0,1,	1,4,3,	4,1,2,
			2,5,4,	6,3,4,	4,7,6,
			7,4,5,	5,8,7
		},
		PrimitiveTopology::TriangleList,
	
		AddMaterial("Resources/uv_grid_2.png")
	
		}
	);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	for (const Material& mat : m_Materials)
	{
		delete mat.pTexture;
	}
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

	WorldToScreen(m_Meshes);
	for (const Mesh& screenMesh : m_Meshes)
	{
		//RENDER LOGIC
		const size_t screenPixels{ static_cast<size_t>(m_Width * m_Height) };
	
		SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
		//logic for topology
		int itterationAmnt{};
		size_t amntOfChecks{};
		if (screenMesh.primitiveTopology == PrimitiveTopology::TriangleList) 
		{
			itterationAmnt = 3;
			amntOfChecks = screenMesh.indices.size();
		}
		else 
		{
			itterationAmnt = 1;
			amntOfChecks = screenMesh.indices.size() - 2;
		}

		for (size_t triIdx = 0; triIdx < amntOfChecks; triIdx += itterationAmnt)
		{
			int idx0{ static_cast<int>(triIdx) + 0 };
			int idx1{ static_cast<int>(triIdx) + 1 };
			int idx2{ static_cast<int>(triIdx) + 2 };

			if (screenMesh.primitiveTopology == PrimitiveTopology::TriangleStrip) {
				if (triIdx % 2 != 0) {
					std::swap(idx0, idx2);
				}
			}

			const Vertex_Out& v0 = screenMesh.vertices_out[screenMesh.indices[idx0]];
			const Vertex_Out& v1 = screenMesh.vertices_out[screenMesh.indices[idx1]];
			const Vertex_Out& v2 = screenMesh.vertices_out[screenMesh.indices[idx2]];

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
			topLeft.x -= 1;
			topLeft.y -= 1;
			bottomRight.x += 1;
			bottomRight.y += 1;

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
							1.f / ( 
								  1.f / v0.position.z * weights.x +
								  1.f / v1.position.z * weights.y +
								  1.f / v2.position.z * weights.z
								  )
						};

						if (m_pDepthBufferPixels[pixelIdx] < hitDepth) { continue; }
						else { m_pDepthBufferPixels[pixelIdx] = hitDepth; }
						switch (m_CurrentRenderMode) {
						case(RenderMode::standard): {
							const Vector2 uv{ (
							v0.uv / v0.position.z * weights.x +
							v1.uv / v1.position.z * weights.y +
							v2.uv / v2.position.z * weights.z) * hitDepth
							};
							finalColor = m_Materials[screenMesh.materialId].pTexture->Sample(uv);
							finalColor.MaxToOne();
							break; }
						case(RenderMode::depth):
							//finalColor = ColorRGB{ hitDepth,hitDepth,hitDepth };
							break;
						}

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

Vector4 dae::Renderer::NdcToScreen(Vector4 ndc) const
{
	ndc.x = (ndc.x + 1) / 2 * m_Width;
	ndc.y = (1 - ndc.y) / 2 * m_Height;
	return ndc;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}



void dae::Renderer::WorldToScreen(std::vector<Mesh>& mesh) const
{
	const float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	for (size_t meshIdx{}; meshIdx < mesh.size(); meshIdx++)
	{
		std::vector<Vertex_Out> temp{};
		for (size_t vertexIdx{}; vertexIdx < mesh[meshIdx].vertices.size(); vertexIdx++)
		{
			const Vector3 view{ m_Camera.viewMatrix.TransformPoint(mesh[meshIdx].vertices[vertexIdx].position)};

			float projectedX{ view.x / view.z };
			float projectedY{ view.y / view.z };
			projectedX /= (aspectRatio * m_Camera.fov);
			projectedY /= m_Camera.fov;

			const Vector4 projected{ projectedX, projectedY, view.z, 0 };
			temp.push_back(Vertex_Out(NdcToScreen(projected), mesh[meshIdx].vertices[vertexIdx].uv, mesh[meshIdx].vertices[vertexIdx].normal, mesh[meshIdx].vertices[vertexIdx].tangent, mesh[meshIdx].vertices[vertexIdx].viewDirection,mesh[meshIdx].vertices[vertexIdx].color));
		}
		mesh[meshIdx].vertices_out = temp;
	}
}
/// <summary>
/// adds material to list and returns the id
/// </summary>
/// <param name="path"> Pathname of the texture </param>
size_t dae::Renderer::AddMaterial(const std::string& path)
{
	m_Materials.push_back(Material{ Texture::LoadFromFile(path) });
	return m_Materials.size() - 1;
}




