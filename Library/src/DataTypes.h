#pragma once
#include "Maths.h"
#include "vector"
#include "Texture.h"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		Vector2 uv{}; //W2
		Vector3 normal{}; //W4
		Vector3 tangent{}; //W4
		Vector3 viewDirection{}; //W4
		ColorRGB color{ ColorRGB{170, 122, 230} };
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
		ColorRGB color{ colors::White };
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	struct DirectionalLight 
	{
		Vector3 dir{};
		ColorRGB color{};
		float intensity{};
	};

	struct Mesh
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };
		
		size_t materialId{};

		std::vector<Vertex_Out> vertices_out{};
		Matrix worldMatrix{};
	};
	struct Material
	{
		Texture* pDiffuse;
		Texture* pNormal;
		Texture* pSpecular;
		Texture* pGloss;

	};

}
