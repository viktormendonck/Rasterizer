#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float speed = 3.f;
		float mouseSpeed = .5f;

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			Matrix finalRotation = Matrix::CreateRotation(totalPitch, totalYaw, 0);
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			invViewMatrix = { right,up,forward,origin };
			viewMatrix = invViewMatrix.Inverse();
			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W3

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			//wasd controlls
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += speed * deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= speed * deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += speed * deltaTime * right;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= speed * deltaTime * right;
			}
			//mouse controlls
			bool leftMouse{ (mouseState & SDL_BUTTON(1)) != 0 };
			bool rightMouse{ (mouseState & SDL_BUTTON(3)) != 0 };

			if (rightMouse && !leftMouse)
			{
				totalPitch += deltaTime * -mouseY * mouseSpeed;
				totalYaw += deltaTime * mouseX * mouseSpeed;
			}

			if (leftMouse && !rightMouse)
			{
				if (mouseY > 0)
				{
					origin -= speed * deltaTime * forward;
				}
				else if (mouseY < 0)
				{
					origin += speed * deltaTime * forward;

				}
				totalYaw += deltaTime * mouseX;
			}

			if (leftMouse && rightMouse)
			{
				if (mouseY > 0)
				{
					origin -= speed * deltaTime * up;
				}
				else if (mouseY < 0)
				{
					origin += speed * deltaTime * up;
				}
			}
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
