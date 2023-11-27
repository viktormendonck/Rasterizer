#pragma once
namespace dae
{
	struct Vector2;
	struct Vector4;
	struct Vector3;
	struct Vector2i
	{
		int x{};
		int y{};

		Vector2i() = default;
		Vector2i(int _x, int _y);

		Vector2 toVector2() const;
		
	};
}
