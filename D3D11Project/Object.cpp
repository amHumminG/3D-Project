#include "Object.h"

#include "OBJ_Loader.h"

void Object::LoadMeshAndMaterial(ID3D11Device* device, const char* pathToOBJFile)
{
	// Load the model
	objl::Loader loader;
	if (!loader.LoadFile(pathToOBJFile))
	{
		std::cerr << "SceneObject::LoadMeshAndMaterial() Failed to load OBJ file!" << std::endl;
	}

	// data to be used for initializing the mesh
	MeshData meshData;

	// vectors to store min and max values for bounding box
	DirectX::XMFLOAT3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
	DirectX::XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	// vertex buffer
	std::vector<Vertex> vertices;
	for (size_t i = 0; i < loader.LoadedVertices.size(); ++i)
	{
		Vertex vertex;

		vertex.Position.x = loader.LoadedVertices[i].Position.X;
		if (vertex.Position.x < min.x) min.x = vertex.Position.x;
		if (vertex.Position.x > max.x) max.x = vertex.Position.x;

		vertex.Position.y = loader.LoadedVertices[i].Position.Y;
		if (vertex.Position.y < min.y) min.y = vertex.Position.y;
		if (vertex.Position.y > max.y) max.y = vertex.Position.y;

		vertex.Position.z = loader.LoadedVertices[i].Position.Z;
		if (vertex.Position.z < min.z) min.z = vertex.Position.z;
		if (vertex.Position.z > max.z) max.z = vertex.Position.z;

		vertex.Normal.x = loader.LoadedVertices[i].Normal.X;
		vertex.Normal.y = loader.LoadedVertices[i].Normal.Y;
		vertex.Normal.z = loader.LoadedVertices[i].Normal.Z;

		vertex.Uv.x = loader.LoadedVertices[i].TextureCoordinate.X;
		vertex.Uv.y = -loader.LoadedVertices[i].TextureCoordinate.Y;

		vertices.push_back(vertex);
	}
	meshData.vertexInfo = { sizeof(Vertex), vertices.size(), vertices.data() };

	// minV and maxV for bounding box
	this->minV = DirectX::XMLoadFloat3(&min);
	this->maxV = DirectX::XMLoadFloat3(&max);

	// index buffer in normal order
	std::vector<unsigned int> indices;
	for (size_t i = 0; i < loader.LoadedIndices.size(); ++i)
	{
		indices.push_back(loader.LoadedIndices[i]);
	}

	meshData.indexInfo = { indices.size(), indices.data() };


	// submesh info
	size_t indexOffset = 0;
	for (size_t i = 0; i < loader.LoadedMeshes.size(); ++i)
	{
		size_t startIndexValue = indexOffset;
		size_t nrOfIndicesInSubMesh = loader.LoadedMeshes[i].Indices.size();

		// TODO: Load textures
		ID3D11ShaderResourceView* ambientTextureSRV = nullptr;
		ID3D11ShaderResourceView* diffuseTextureSRV = nullptr;
		ID3D11ShaderResourceView* specularTextureSRV = nullptr;

		meshData.subMeshInfo.push_back({ startIndexValue, nrOfIndicesInSubMesh, ambientTextureSRV, diffuseTextureSRV, specularTextureSRV });

		indexOffset += loader.LoadedMeshes[i].Indices.size();
	}

	this->mesh.Initialize(device, meshData);

	// -------------------------- MATERIAL --------------------------
	// TODO: for now we only support one material per object but we should support one material per submesh
	Material material;
	material.ambient = loader.LoadedMaterials[0].Ka.X;
	material.diffuse = loader.LoadedMaterials[0].Kd.X;
	material.specular = loader.LoadedMaterials[0].Ks.X;
	material.shininess = loader.LoadedMaterials[0].Ns;
	this->materialConstantBuffer.Initialize(device, sizeof(Material), &material);
}
