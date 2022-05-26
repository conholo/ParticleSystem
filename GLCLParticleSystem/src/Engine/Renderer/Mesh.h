#pragma once


namespace Engine
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
			:m_Vertices(vertices), m_Indices(indices) { }

		std::vector<Vertex>& GetVertices() { return m_Vertices; }
		std::vector<uint32_t>& GetIndices() { return m_Indices; }
		const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};

	class MeshFactory
	{
	public:
		static Mesh* Sphere(float radius);
	};
}