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
		//Vector3 normal{}; //W4
		//Vector3 tangent{}; //W4
		//Vector3 viewDirection{}; //W4
		ColorRGB color{colors::White};
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector2 uv{};
		//Vector3 normal{};
		//Vector3 tangent{};
		//Vector3 viewDirection{};
		ColorRGB color{ colors::White };
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
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
		Texture* pTexture;

	};

}
