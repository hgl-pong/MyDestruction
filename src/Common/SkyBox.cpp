#include "XUtil.h"
#include "SkyBox.h"
#include "DXTrace.h"
#include "../Importer/MeshImporter.h"
using namespace DirectX;
using namespace Graphics;
struct InstancedData
{
    XMMATRIX world;
    XMMATRIX worldInvTranspose;
};


void SkyBox::SetModel(const Model* pModel)
{
    m_pModel = pModel;
}



void SkyBox::Draw(ID3D11DeviceContext * deviceContext, IEffect * effect)
{

        IEffectMeshData* pEffectMeshData = dynamic_cast<IEffectMeshData*>(effect);
		if (!pEffectMeshData)
			return;

        IEffectMaterial* pEffectMaterial = dynamic_cast<IEffectMaterial*>(effect);
        if (pEffectMaterial)
            pEffectMaterial->SetMaterial(m_pModel->materials[0]);

        IEffectTransform* pEffectTransform = dynamic_cast<IEffectTransform*>(effect);
        if (pEffectTransform)
            pEffectTransform->SetWorldMatrix(m_Transform.GetLocalToWorldMatrixXM());

        effect->Apply(deviceContext);

        MeshDataInput input = pEffectMeshData->GetInputData(m_pModel->meshData);
        {
            deviceContext->IASetVertexBuffers(0, (uint32_t)input.pVertexBuffers.size(), 
                input.pVertexBuffers.data(), input.strides.data(), input.offsets.data());
            deviceContext->IASetIndexBuffer(input.pIndexBuffer, input.indexCount > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);

            deviceContext->DrawIndexed(input.indexCount, 0, 0);
        }
        
    
}


