#include "Vector2i.h"
#include <cassert>

#include "Vector4.h"
#include <cmath>

#include "MathHelpers.h"
#include "Vector2.h"
dae::Vector2i::Vector2i(int _x, int _y)
	: x{_x}
	, y{_y}
{
}

dae::Vector2 dae::Vector2i::toVector2() const
{
	return Vector2(static_cast<float>(x),static_cast<float>(y));
}
