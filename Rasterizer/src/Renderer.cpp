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
	m_Camera.Initialize(45.f, { 0.f, 5.f, -64.f });
	


	
	m_Meshes.push_back(
		Mesh{
			{},{},PrimitiveTopology::TriangleList,AddMaterial("Resources/vehicle_diffuse.png","Resources/vehicle_normal.png","Resources/vehicle_specular.png","Resources/vehicle_gloss.png")
		}
	);
	
	Utils::ParseOBJ("Resources/vehicle.obj", m_Meshes[0].vertices,m_Meshes[0].indices);

}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	for (const Material& mat : m_Materials)
	{
		delete mat.pDiffuse;
		delete mat.pGloss;
		delete mat.pNormal;
		delete mat.pSpecular;
	}
}

void Renderer::Update(Timer* pTimer)
{
	const float deltaTime{ pTimer->GetElapsed() };
	m_Camera.Update(pTimer);
	if (m_isRotating) {
		m_Meshes[0].worldMatrix *= Matrix::CreateRotationY(deltaTime * PI_DIV_4);
	}
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
		int iteration{};
		size_t amntOfChecks{};

		if (screenMesh.primitiveTopology == PrimitiveTopology::TriangleList) 
		{
			iteration = 3;
			amntOfChecks = screenMesh.indices.size();
		}
		else 
		{
			iteration = 1;
			amntOfChecks = screenMesh.indices.size() - 2;
		}

		for (size_t triIdx = 0; triIdx < amntOfChecks; triIdx += iteration)
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
			if (v0.position.z < 0 || v0.position.z > 1        ||
				v1.position.z < 0 || v1.position.z > 1        ||
				v2.position.z < 0 || v2.position.z > 1
				)
			{
				continue;
			}
			switch (m_CurrentCullingMode)
			{
			case dae::Renderer::CullingMode::partial:
				if ((v0.position.x < 0 || v0.position.x > m_Width || v0.position.y < 0 || v0.position.y > m_Height) &&
					(v1.position.x < 0 || v1.position.x > m_Width || v1.position.y < 0 || v1.position.y > m_Height) &&
					(v2.position.x < 0 || v2.position.x > m_Width || v2.position.y < 0 || v2.position.y > m_Height))
				{
					continue;
				}
				break;
			case dae::Renderer::CullingMode::complete:
				if (v0.position.x < 0 || v0.position.x > m_Width ||
					v1.position.x < 0 || v1.position.x > m_Width ||
					v2.position.x < 0 || v2.position.x > m_Width ||
					v0.position.y < 0 || v0.position.y > m_Height ||
					v1.position.y < 0 || v1.position.y > m_Height ||
					v2.position.y < 0 || v2.position.y > m_Height) 
				{
					continue;
				}
				break;
			case dae::Renderer::CullingMode::off:
				break;
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
						const float linearHitDepth{
							1.f / ( 
								  1.f / v0.position.w * weights.x +
								  1.f / v1.position.w * weights.y +
								  1.f / v2.position.w * weights.z
								  )
						};
						float nonLinearHitDepth{
							1.f / (
								(1.f / v0.position.z) * weights.x +
								(1.f / v1.position.z) * weights.y +
								(1.f / v2.position.z) * weights.z
								)
						};
						if (m_pDepthBufferPixels[pixelIdx] < linearHitDepth) { continue; }
						else { m_pDepthBufferPixels[pixelIdx] = linearHitDepth; }
						switch (m_CurrentRenderMode) {
						case(RenderMode::standard): {
#pragma region Interpolation
							const Vector4 pos{
								static_cast<float>(px),
								static_cast<float>(py),
								nonLinearHitDepth,
								linearHitDepth
							};
							const Vector2 uv{ (
							v0.uv / v0.position.w * weights.x +
							v1.uv / v1.position.w * weights.y +
							v2.uv / v2.position.w * weights.z) * linearHitDepth
							};
							const Vector3 normal{ ((
							v0.normal / v0.position.w * weights.x +
							v1.normal / v1.position.w * weights.y +
							v2.normal / v2.position.w * weights.z) * linearHitDepth).Normalized()
							};
							const Vector3 tangent{ ((
							v0.tangent / v0.position.w * weights.x +
							v1.tangent / v1.position.w * weights.y +
							v2.tangent / v2.position.w * weights.z) * linearHitDepth).Normalized()
							};
							const Vector3 viewDir{ ((
							v0.viewDirection / v0.position.w * weights.x +
							v1.viewDirection / v1.position.w * weights.y +
							v2.viewDirection / v2.position.w * weights.z) * linearHitDepth).Normalized()
							};
#pragma endregion
							const Vertex_Out out(pos,uv,normal,tangent,viewDir,finalColor);
							finalColor = PixelShading(out,screenMesh.materialId);
							finalColor.MaxToOne();
							break; }

						case(RenderMode::depth): {
							
							nonLinearHitDepth = Utils::Remap(0.995f, 1.f, 0.f, 1.f, nonLinearHitDepth);
							finalColor = ColorRGB{ nonLinearHitDepth,nonLinearHitDepth,nonLinearHitDepth };
							finalColor.MaxToOne();
							break; }
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



ColorRGB dae::Renderer::PixelShading(const Vertex_Out& vert, int matId)
{
	const DirectionalLight light = {
				{ .577f, -.577f, .577f },
				ColorRGB{1,1,1},
				7.f
	};
	const ColorRGB ambientColor{ 0.03f,0.03f,0.03f };
	const float shine{ 25.f };
	
	const ColorRGB sampledNormal{ m_Materials[matId].pNormal->Sample(vert.uv) };
	const ColorRGB sampledDiffuse{ m_Materials[matId].pDiffuse->Sample(vert.uv) };
	const ColorRGB sampledGloss{ m_Materials[matId].pGloss->Sample(vert.uv) };
	const ColorRGB sampledSpecular{ m_Materials[matId].pSpecular->Sample(vert.uv) };
	
	const Vector3 biNormal{Vector3::Cross(vert.normal,vert.tangent)};
	const Matrix tangentAxis{vert.tangent,biNormal,vert.normal,Vector3::Zero };
	
	
	Vector3 sampledNormalVector{ sampledNormal.r,sampledNormal.g,sampledNormal.b };
	Vector3 normal{ sampledNormalVector * 2.f - Vector3{1,1,1} };

	if (m_UsingNormalMap)
	{
		normal = tangentAxis.TransformPoint(normal.Normalized()).Normalized();
	}
	else
	{
		normal = vert.normal;
	}
	
	const ColorRGB diffuse{ dae::BRDF::Lambert(7,sampledDiffuse) };

	const float observedArea{ Vector3::Dot(normal, -light.dir) };
	if (observedArea < 0) {
		return colors::Black;
	}
	
	
	
	const ColorRGB gloss{ sampledGloss * shine };
	
	const ColorRGB specular{ dae::BRDF::Phong(
		m_Materials[matId].pSpecular->Sample(vert.uv),
		gloss,
		light.dir,
		vert.viewDirection,
		normal
	) };
	
	switch (m_CurrentShadingMode)
	{
	case dae::Renderer::ShadingMode::Combined:
		return ((light.color  * diffuse) +ambientColor + specular) * observedArea;
		break;
	case dae::Renderer::ShadingMode::ObservedArea:
		return ColorRGB{ observedArea,observedArea,observedArea };
		break;
	case dae::Renderer::ShadingMode::Diffuse:
		return (light.color ) * diffuse * observedArea;
		break;
	case dae::Renderer::ShadingMode::Specular:
		return specular;
		break;
	}
	
	
}


bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}



void dae::Renderer::WorldToScreen(std::vector<Mesh>& mesh) const
{
	const float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	const Matrix projectionMatrix{ 
		Matrix(	Vector4(1 / (aspectRatio * m_Camera.fov) , 0 , 0 , 0),
				Vector4(0 , 1 / m_Camera.fov , 0 , 0),
				Vector4(0 , 0	, m_Camera.far / (m_Camera.far - m_Camera.near), 1),
				Vector4(0 , 0	, -(m_Camera.far * m_Camera.near) / (m_Camera.far - m_Camera.near) , 0))};
	for (size_t meshIdx{}; meshIdx < mesh.size(); meshIdx++)
	{
		std::vector<Vertex_Out> temp{};
		for (size_t vertexIdx{}; vertexIdx < mesh[meshIdx].vertices.size(); vertexIdx++)
		{
			const Matrix wvp{ mesh[meshIdx].worldMatrix * m_Camera.viewMatrix * projectionMatrix };
			Vector4 projectedView{ wvp.TransformPoint({mesh[meshIdx].vertices[vertexIdx].position,1}) };
			projectedView.x /= projectedView.w;
			projectedView.y /= projectedView.w;
			projectedView.z /= projectedView.w;
			const Vector3 normal{ mesh[meshIdx].worldMatrix.TransformVector(mesh[meshIdx].vertices[vertexIdx].normal).Normalized() };
			const Vector3 tangent{ mesh[meshIdx].worldMatrix.TransformVector(mesh[meshIdx].vertices[vertexIdx].tangent).Normalized() };
			const Vector3 viewDir{ (mesh[meshIdx].worldMatrix.TransformPoint(mesh[meshIdx].vertices[vertexIdx].position) - m_Camera.origin).Normalized() };

			temp.push_back(Vertex_Out(NdcToScreen(projectedView), 
								mesh[meshIdx].vertices[vertexIdx].uv, 
								normal,	
								tangent, 
								viewDir,
								mesh[meshIdx].vertices[vertexIdx].color));
		}
		mesh[meshIdx].vertices_out = temp;
	}
}
/// <summary>
/// adds material to list and returns the id
/// </summary>
/// <param name="path"> Pathname of the texture </param>
size_t dae::Renderer::AddMaterial(const std::string& diffuse , const std::string& normal , const std::string& specular ,const std::string& gloss)
{
	Texture* diffuseTexture{ nullptr };
	if (!diffuse.empty()) diffuseTexture = Texture::LoadFromFile(diffuse);

	Texture* normalTexture{ nullptr };
	if (!normal.empty()) normalTexture = Texture::LoadFromFile(normal);

	Texture* specularTexture{ nullptr };
	if (!specular.empty()) specularTexture = Texture::LoadFromFile(specular);

	Texture* glossTexture{ nullptr };
	if (!gloss.empty()) glossTexture = Texture::LoadFromFile(gloss);


	m_Materials.push_back(Material{ diffuseTexture, normalTexture, specularTexture, glossTexture });
	return m_Materials.size() - 1;
}

void dae::Renderer::CycleRenderMode()
{
	m_CurrentRenderMode = static_cast<RenderMode>((static_cast<int>(m_CurrentRenderMode) + 1) % 2);
}

void dae::Renderer::CycleCullingMode()
{
	m_CurrentCullingMode = static_cast<CullingMode>((static_cast<int>(m_CurrentCullingMode) + 1) % 3);
}

void dae::Renderer::CycleShadingMode()
{
	m_CurrentShadingMode = static_cast<ShadingMode>((static_cast<int>(m_CurrentShadingMode) + 1) % 4);
}

void dae::Renderer::ToggleSpin()
{
	m_isRotating = !m_isRotating;
}

void dae::Renderer::ToggleNormalMap()
{
	m_UsingNormalMap = !m_UsingNormalMap;
}


