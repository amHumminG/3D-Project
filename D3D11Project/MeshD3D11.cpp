#include "MeshD3D11.h"

void MeshD3D11::Initialize(ID3D11Device* device, const MeshData& meshInfo)
{
	vertexBuffer.Initialize(device, meshInfo.vertexInfo.sizeOfVertex, meshInfo.vertexInfo.nrOfVerticesInBuffer, meshInfo.vertexInfo.vertexData);
	indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, meshInfo.indexInfo.indexData);

	subMeshes.reserve(meshInfo.subMeshInfo.size());
	for (int i = 0; i < meshInfo.subMeshInfo.size(); i++)
	{
		const MeshData::SubMeshInfo subMesh = meshInfo.subMeshInfo[i];
		SubMeshD3D11 newSubMesh;
		newSubMesh.Initialize(subMesh.startIndexValue, subMesh.nrOfIndicesInSubMesh, subMesh.ambientTextureSRV, subMesh.diffuseTextureSRV, subMesh.specularTextureSRV);
		subMeshes.push_back(std::move(newSubMesh));
	}
}

void MeshD3D11::BindMeshBuffers(ID3D11DeviceContext* context) const
{
	UINT offset = 0;	// Offset in bytes from the start of the buffer to the first element to use
	UINT stride = vertexBuffer.GetVertexSize();	// The size of a single vertex in the buffer
	ID3D11Buffer* IA_VertexBuffers[1] = { vertexBuffer.GetBuffer() };
	context->IASetVertexBuffers(0, 1, IA_VertexBuffers, &stride, &offset);

	context->IASetIndexBuffer(indexBuffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
}

void MeshD3D11::PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) const
{
	subMeshes[subMeshIndex].PerformDrawCall(context);
}


// ----------------- getters -----------------

size_t MeshD3D11::GetNrOfSubMeshes() const
{
	return subMeshes.size();
}

ID3D11ShaderResourceView* MeshD3D11::GetAmbientSRV(size_t subMeshIndex) const
{
	return subMeshes[subMeshIndex].GetAmbientSRV();
}

ID3D11ShaderResourceView* MeshD3D11::GetDiffuseSRV(size_t subMeshIndex) const
{
	return subMeshes[subMeshIndex].GetDiffuseSRV();
}

ID3D11ShaderResourceView* MeshD3D11::GetSpecularSRV(size_t subMeshIndex) const
{
	return subMeshes[subMeshIndex].GetSpecularSRV();
}
