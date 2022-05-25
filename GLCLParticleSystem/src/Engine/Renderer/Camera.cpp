#include "glclpch.h"
#include "Engine/Renderer/Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define PI 3.14159265359


namespace Engine
{
	Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
		:m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		m_Position = CalculatePosition();

		glm::quat orientation = CalculateOrientation();
		m_WorldRotationEulers = glm::degrees(glm::eulerAngles(orientation));

		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);

		RecalculatePerspectiveProjection();
	}

	Camera::Camera()
		:m_FOV(45.0f), m_AspectRatio(1920.0f / 1080.0f), m_NearClip(0.1f), m_FarClip(1000.0f)
	{
		m_Position = CalculatePosition();

		glm::quat orientation = CalculateOrientation();
		m_WorldRotationEulers = glm::degrees(glm::eulerAngles(orientation));

		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);

		RecalculatePerspectiveProjection();
	}

	void Camera::SetPerspective()
	{
		RecalculatePerspectiveProjection();
		m_PitchDelta = 0.0f;
		m_YawDelta = 0.0f;
		m_Pitch = 0.0f;
		m_Yaw = 0.0f;
		m_PositionDelta = glm::vec3(0.0f);
		m_Position = { 0.0f, 0.0f, 10.0f };
	}

	glm::vec3 Camera::CalculatePosition() const
	{
		return m_FocalPoint - Forward() * m_DistanceFromFocalPoint + m_PositionDelta;
	}

	glm::quat Camera::CalculateOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void Camera::RecalculatePerspectiveView()
	{
		const float yawSign = Up().y < 0.0f ? -1.0f : 1.0f;

		const glm::vec3 lookAt = m_Position + Forward();
		m_WorldRotationEulers = glm::normalize(m_FocalPoint - m_Position);
		m_FocalPoint = m_Position + Forward() * m_DistanceFromFocalPoint;
		m_DistanceFromFocalPoint = glm::distance(m_Position, m_FocalPoint);
		m_ViewMatrix = glm::lookAt(m_Position, lookAt, glm::vec3{ 0.0f, yawSign, 0.0f });

		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	void Camera::RecalculatePerspectiveProjection()
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void Camera::SetViewportSize(float width, float height)
	{
		m_AspectRatio = width / height;
		RecalculatePerspectiveProjection();
	}

	void Camera::SetRotation(const glm::vec2& rotation)
	{
		m_Pitch = glm::radians(rotation.x);
		m_Yaw = glm::radians(rotation.y);
		RecalculatePerspectiveView();
	}

	glm::vec3 Camera::Forward() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 Camera::Up() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Camera::Right() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}
}