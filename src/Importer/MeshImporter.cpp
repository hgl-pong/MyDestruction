#include "MeshImporter.h"
#include "../Common/Transform.h"
#include "../Common/Texture.h"
#include "../Common/XUtil.h"
#include "../Common/Application.h"
#include "TextureImporter.h"


#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cfileio.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <Assimp/cimport.h>
#include "../Common/XUtil.h"
#include "MeshImporter.h"
#include "TextureImporter.h"

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace DirectX;

namespace
{
    // ModelManager单例
    MeshImporter* s_pInstance = nullptr;
}


MeshImporter::MeshImporter(Application*application)
{
    name = "Mesh Importer";
    app = application;
    if (s_pInstance)
        throw std::exception("MeshImporter is a singleton!");
    s_pInstance = this;
}

MeshImporter::~MeshImporter()
{
}

MeshImporter& MeshImporter::Get()
{
    if (!s_pInstance)
        throw std::exception("MeshImporter needs an instance!");
    return *s_pInstance;
}

bool MeshImporter::Init(ID3D11Device* device)
{
    m_pDevice = device;
    m_pDevice->GetImmediateContext(m_pDeviceContext.ReleaseAndGetAddressOf());
    return true;
}

const Model* MeshImporter::CreateFromFile(std::string_view filename)
{
    XID modelID = StringToID(filename);
    if (m_Models.count(modelID))
        return &m_Models[modelID];

    using namespace Assimp;
    namespace fs = std::filesystem;
    Importer importer;

    auto pAssimpScene = importer.ReadFile(filename.data(), aiProcess_ConvertToLeftHanded |
        aiProcess_Triangulate | aiProcess_ImproveCacheLocality);

    if (pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->HasMeshes())
    {
        auto& model = m_Models[modelID];

        model.meshDatas.resize(pAssimpScene->mNumMeshes);
        model.materials.resize(pAssimpScene->mNumMaterials);
        for (uint32_t i = 0; i < pAssimpScene->mNumMeshes; ++i)
        {
            auto& mesh = model.meshDatas[i];

            auto pAiMesh = pAssimpScene->mMeshes[i];
            uint32_t numVertices = pAiMesh->mNumVertices;

            CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
            D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
            // 位置
            if (pAiMesh->mNumVertices > 0)
            {
                initData.pSysMem = pAiMesh->mVertices;
                bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
                m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh.m_pVertices.GetAddressOf());

                BoundingBox::CreateFromPoints(mesh.m_BoundingBox, numVertices,
                    (const XMFLOAT3*)pAiMesh->mVertices, sizeof(XMFLOAT3));
                if (i == 0)
                    model.boundingbox = mesh.m_BoundingBox;
                else
                    model.boundingbox.CreateMerged(model.boundingbox, model.boundingbox, mesh.m_BoundingBox);

                model.meshdata.vertices.resize(numVertices);
                memcpy_s(model.meshdata.vertices.data(), sizeof(XMFLOAT3) * numVertices, (const XMFLOAT3*)pAiMesh->mVertices, sizeof(XMFLOAT3) * numVertices);
            }

            // 法线
            if (pAiMesh->HasNormals())
            {
                initData.pSysMem = pAiMesh->mNormals;
                bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
                m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh.m_pNormals.GetAddressOf());

                model.meshdata.normals.resize(numVertices);
                memcpy_s(model.meshdata.normals.data(), sizeof(XMFLOAT3) * numVertices, (const XMFLOAT3*)pAiMesh->mNormals, sizeof(XMFLOAT3) * numVertices);
            }

            // 切线
            if (pAiMesh->HasTangentsAndBitangents())
            {
                std::vector<XMFLOAT4> tangents(numVertices, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
                for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
                {
                    memcpy_s(&tangents[i], sizeof(XMFLOAT3),
                        pAiMesh->mTangents + i, sizeof(XMFLOAT3));
                }

                initData.pSysMem = tangents.data();
                bufferDesc.ByteWidth = pAiMesh->mNumVertices * sizeof(XMFLOAT4);
                m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh.m_pTangents.GetAddressOf());

                model.meshdata.tangents.resize(numVertices);
                memcpy_s(model.meshdata.tangents.data(), sizeof(XMFLOAT3) * numVertices, (const XMFLOAT3*)pAiMesh->mTangents, sizeof(XMFLOAT3) * numVertices);
            }

            // 纹理坐标
            uint32_t numUVs = 8;
            while (numUVs && !pAiMesh->HasTextureCoords(numUVs - 1))
                numUVs--;

            if (numUVs > 0)
            {
                mesh.m_pTexcoordArrays.resize(numUVs);

                for (uint32_t i = 0; i < numUVs; ++i)
                {
                    std::vector<XMFLOAT2> uvs(numVertices);
                    model.meshdata.texcoords.resize(numVertices);
                    for (uint32_t j = 0; j < numVertices; ++j)
                    {
                        memcpy_s(&uvs[j], sizeof(XMFLOAT2),
                            pAiMesh->mTextureCoords[i] + j, sizeof(XMFLOAT2));

                        memcpy_s(&model.meshdata.texcoords[j], sizeof(XMFLOAT2),
                            pAiMesh->mTextureCoords[i] + j, sizeof(XMFLOAT2));
                    }
                    initData.pSysMem = uvs.data();
                    bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT2);
                    m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh.m_pTexcoordArrays[i].GetAddressOf());
                }
                
            }

            // 索引
            uint32_t numFaces = pAiMesh->mNumFaces;
            uint32_t numIndices = numFaces * 3;
            if (numFaces > 0)
            {
                mesh.m_IndexCount = numIndices;
                //if (numIndices < 65535)
                //{
                //    std::vector<uint16_t> indices(numIndices);
                //    for (size_t i = 0; i < numFaces; ++i)
                //    {
                //        indices[i * 3] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[0]);
                //        indices[i * 3 + 1] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[1]);
                //        indices[i * 3 + 2] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[2]);
                //    }
                //    bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
                //    initData.pSysMem = indices.data();
                //    m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());
                //}
                //else
                {
                    std::vector<uint32_t> indices(numIndices);
                    for (size_t i = 0; i < numFaces; ++i)
                    {
                        memcpy_s(indices.data() + i * 3, sizeof(uint32_t) * 3,
                            pAiMesh->mFaces[i].mIndices, sizeof(uint32_t) * 3);
                    }
                    bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
                    initData.pSysMem = indices.data();
                    m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh.m_pIndices.GetAddressOf());

                    model.meshdata.indices32 = indices;
                }
            }

            // 材质索引
            mesh.m_MaterialIndex = pAiMesh->mMaterialIndex;
        }


        for (uint32_t i = 0; i < pAssimpScene->mNumMaterials; ++i)
        {
            auto& material = model.materials[i];
            material = new Material();
            auto pAiMaterial = pAssimpScene->mMaterials[i];
            XMFLOAT4 vec{};
            float value{};
            uint32_t boolean{};
            uint32_t num = 3;

            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, (float*)&vec, &num))
                material->Set("$AmbientColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, (float*)&vec, &num))
                material->Set("$DiffuseColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, (float*)&vec, &num))
                material->Set("$SpecularColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, (float*)&vec, &num))
                material->Set("$EmissiveColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, (float*)&vec, &num))
                material->Set("$TransparentColor", vec);
            if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, (float*)&vec, &num))
                material->Set("$ReflectiveColor", vec);
            if (pAiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString aiPath;
                pAiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);
                fs::path tex_filename = filename;
                tex_filename = tex_filename.parent_path() / aiPath.C_Str();
                TextureImporter::Get().CreateTexture(tex_filename.string(), true, true);
                material->SetTexture("$Diffuse", tex_filename.string());
            }
            if (pAiMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
            {
                aiString aiPath;
                pAiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiPath);
                fs::path tex_filename = filename;
                tex_filename = tex_filename.parent_path() / aiPath.C_Str();
                TextureImporter::Get().CreateTexture(tex_filename.string());
                material->SetTexture("$Normal", tex_filename.string());
            }
        }

        return &model;
    }

   // auto pAssimpScene = importer.ReadFile(filename.data(), aiProcess_ConvertToLeftHanded |
   //      aiProcess_Triangulate | aiProcess_ImproveCacheLocality);

   // if (pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->HasMeshes())
   // {
   //     auto& model = m_Models[modelID];
   //     model.m_pMesh->name = filename;
   //     //model.m_pMesh->m_pDestructionMesh.resize(pAssimpScene->mNumMeshes);
   //     model.m_pMesh->m_pDestructionMesh = std::vector<DestructionMesh*>(pAssimpScene->mNumMeshes);
   //     model.materials = std::vector < Material*> (pAssimpScene->mNumMaterials);

   //     for (uint32_t i = 0; i < pAssimpScene->mNumMeshes; ++i)
   //     {
			//model.m_pMesh->m_pDestructionMesh[i] = new DestructionMesh();
   //         auto& mesh = model.m_pMesh->m_pDestructionMesh[i];

   //         auto pAiMesh = pAssimpScene->mMeshes[i];
   //         uint32_t numVertices = pAiMesh->mNumVertices;

   //         CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
   //         D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
   //         // 位置
   //         if (pAiMesh->mNumVertices > 0)
   //         {
   //             initData.pSysMem = pAiMesh->mVertices;
			//	mesh->m_pMeshData.m_VertexCount = pAiMesh->mNumVertices;
			//	NvcVec3* vertices = (NvcVec3*)pAiMesh->mVertices;
   //             mesh->vertices = new NvcVec3[numVertices];
   //             memcpy_s(mesh->vertices, sizeof(NvcVec3)*mesh->m_pMeshData.m_VertexCount,
   //                 pAiMesh->mVertices, sizeof(aiVector3D) * mesh->m_pMeshData.m_VertexCount);
			//	for (size_t i = 0; i < numVertices; ++i) {
			//		if (mesh->vertices[i].x != pAiMesh->mVertices[i].x && mesh->vertices[i].y != pAiMesh->mVertices[i].y && mesh->vertices[i].z != pAiMesh->mVertices[i].z)
   //                     Logger::Debug("debug", "不相等");
			//	}
   //             bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
   //             m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh->m_pMeshData.m_pVertices.GetAddressOf());
   //             BoundingBox::CreateFromPoints(mesh->m_pMeshData.m_BoundingBox, numVertices,
   //                 (const XMFLOAT3*)pAiMesh->mVertices, sizeof(XMFLOAT3));
   //             if (i == 0)
   //                 model.boundingbox = mesh->m_pMeshData.m_BoundingBox;
   //             else
   //                 model.boundingbox.CreateMerged(model.boundingbox, model.boundingbox, mesh->m_pMeshData.m_BoundingBox);
   //         }

   //         // 法线
   //         if (pAiMesh->HasNormals())
   //         {
   //             initData.pSysMem = pAiMesh->mNormals;
   //             mesh->normals = new NvcVec3[numVertices];
			//	memcpy_s(mesh->normals, sizeof(NvcVec3) * mesh->m_pMeshData.m_VertexCount,
   //                 pAiMesh->mNormals, sizeof(aiVector3D) * mesh->m_pMeshData.m_VertexCount);
   //             bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT3);
   //             m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh->m_pMeshData.m_pNormals.GetAddressOf());
   //         }

   //         // 切线
   //         if (pAiMesh->HasTangentsAndBitangents())
   //         {
   //             std::vector<XMFLOAT4> tangents(numVertices, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
   //             for (uint32_t i = 0; i < pAiMesh->mNumVertices; ++i)
   //             {
   //                 memcpy_s(&tangents[i], sizeof(XMFLOAT3),
   //                     pAiMesh->mTangents + i, sizeof(XMFLOAT3));
   //             }

   //             initData.pSysMem = tangents.data();
   //             mesh->tangents = (float*)tangents.data();
   //             bufferDesc.ByteWidth = pAiMesh->mNumVertices * sizeof(XMFLOAT4);
   //             m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh->m_pMeshData.m_pTangents.GetAddressOf());
   //         }

   //         // 纹理坐标
   //         uint32_t numUVs = 8;
   //         while (numUVs && !pAiMesh->HasTextureCoords(numUVs - 1))
   //             numUVs--;
			//NvcVec2* uv = new NvcVec2[numUVs * numVertices*2];
			//if (numUVs > 0)
			//{
			//	mesh->m_pMeshData.m_pTexcoordArrays.resize(numUVs);
			//	for (uint32_t i = 0; i < numUVs; ++i)
			//	{
			//		std::vector<XMFLOAT2> uvs(numVertices);
			//		for (uint32_t j = 0; j < numVertices; ++j)
			//		{
			//			uv[i * numUVs + j].x = pAiMesh->mTextureCoords[i]->x + j;
			//			uv[i * numUVs + j].y = pAiMesh->mTextureCoords[i]->y + j;
			//			memcpy_s(&uvs[j], sizeof(XMFLOAT2),
			//				pAiMesh->mTextureCoords[i] + j, sizeof(XMFLOAT2));

			//		}
   //                 initData.pSysMem = uvs.data();
   //                 bufferDesc.ByteWidth = numVertices * sizeof(XMFLOAT2);
   //                 m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh->m_pMeshData.m_pTexcoordArrays[i].GetAddressOf());
   //             }
   //         }
   //         mesh->texture_coords = (NvcVec2*)uv;
   //         // 索引
   //         uint32_t numFaces = pAiMesh->mNumFaces;
   //        
   //         uint32_t numIndices = numFaces * 3;
   //         mesh->m_pMeshData.m_IndexCount = numFaces * 3;
   //         UINT* m_indices = nullptr;
   //         if (numFaces > 0)
   //         {
   //             m_indices = new UINT[numIndices];
   //             mesh->m_pMeshData.m_IndexCount = numIndices;
   //             if (numIndices < 65535)
   //             {
			//		for (UINT j = 0; j < pAiMesh->mNumFaces; ++j)
			//		{
			//			if (pAiMesh->mFaces[j].mNumIndices != 3)
			//			{
			//			}
			//			else
			//			{
			//				memcpy(&m_indices[j * 3], pAiMesh->mFaces[j].mIndices, 3 * sizeof(uint));
			//			}
			//		}
   //                 std::vector<uint16_t> indices(numIndices);
   //                 std::vector<UINT> indices32(numIndices);
   //                 for (size_t i = 0; i < numFaces; ++i)
   //                 {
   //                     indices[i * 3] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[0]);
   //                     indices[i * 3 + 1] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[1]);
   //                     indices[i * 3 + 2] = static_cast<uint16_t>(pAiMesh->mFaces[i].mIndices[2]);
			//			indices32[i * 3] = static_cast<UINT>(pAiMesh->mFaces[i].mIndices[0]);
			//			indices32[i * 3 + 1] = static_cast<UINT>(pAiMesh->mFaces[i].mIndices[1]);
			//			indices32[i * 3 + 2] = static_cast<UINT>(pAiMesh->mFaces[i].mIndices[2]);
   //                 }
   //                 bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
   //                 initData.pSysMem = indices.data();
   //                 mesh->indices = new UINT[numIndices];
   //                 memcpy_s(mesh->indices,sizeof(UINT)*numIndices,indices32.data(),sizeof(UINT)*numIndices);
   //                 m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh->m_pMeshData.m_pIndices.GetAddressOf());
			//		//mesh->blast_mesh = NvBlastExtAuthoringCreateMesh((NvcVec3*)pAiMesh->mVertices, (NvcVec3*)pAiMesh->mNormals, uv, (UINT)pAiMesh->mNumVertices,
			//		//	m_indices, (UINT)pAiMesh->mNumFaces * 3);
   //             }
   //             else
   //             {
   //                 std::vector<uint32_t> indices(numIndices);
   //                 for (size_t i = 0; i < numFaces; ++i)
   //                 {
   //                     memcpy_s(indices.data() + i * 3, sizeof(uint32_t) * 3,
   //                         pAiMesh->mFaces[i].mIndices, sizeof(uint32_t) * 3);
   //                 }
   //                 bufferDesc = CD3D11_BUFFER_DESC(numIndices * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
   //                 initData.pSysMem = indices.data();
			//		mesh->indices = new UINT[numIndices];
			//		memcpy_s(mesh->indices, sizeof(UINT), indices.data(), sizeof(UINT));
   //                 m_pDevice->CreateBuffer(&bufferDesc, &initData, mesh->m_pMeshData.m_pIndices.GetAddressOf());
   //             }
   //         }
			////mesh->blast_mesh = NvBlastExtAuthoringCreateMesh((NvcVec3*)pAiMesh->mVertices, (NvcVec3*)pAiMesh->mNormals, uv, (UINT)pAiMesh->mNumVertices,
			////	(UINT*)pAiMesh->mFaces, (UINT)pAiMesh->mNumFaces * 3);
   //         // 材质索引
   //         mesh->m_pMeshData.m_MaterialIndex = pAiMesh->mMaterialIndex;
			//mesh->chunk_id = i;
			////mesh->chunk_depth = 0;
   //         mesh->mesh_id = modelID + mesh->chunk_id;
   //         mesh->name = "chunk" + std::to_string(mesh->chunk_id);

   //     }


   //     for (uint32_t i = 0; i < pAssimpScene->mNumMaterials; ++i)
   //     {
			//model.materials[i] = new Material();
   //         auto& material = model.materials[i];

   //         auto pAiMaterial = pAssimpScene->mMaterials[i];
   //         XMFLOAT4 vec{};
   //         float value{};
   //         uint32_t boolean{};
   //         uint32_t num = 3;

   //         if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, (float*)&vec, &num))
   //             material->Set("$AmbientColor", vec);
   //         if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, (float*)&vec, &num))
   //             material->Set("$DiffuseColor", vec);
   //         if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, (float*)&vec, &num))
   //             material->Set("$SpecularColor", vec);
   //         if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, (float*)&vec, &num))
   //             material->Set("$EmissiveColor", vec);
   //         if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, (float*)&vec, &num))
   //             material->Set("$TransparentColor", vec);
   //         if (aiReturn_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, (float*)&vec, &num))
   //             material->Set("$ReflectiveColor", vec);
   //         if (pAiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
   //         {
   //             aiString aiPath;
   //             pAiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);
   //             fs::path tex_filename = filename;
   //             tex_filename = tex_filename.parent_path() / aiPath.C_Str();
   //             TextureImporter::Get().CreateTexture(tex_filename.string(), true, true);
   //             material->SetTexture("$Diffuse", tex_filename.string());
   //         }
   //         if (pAiMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
   //         {
   //             aiString aiPath;
   //             pAiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiPath);
   //             fs::path tex_filename = filename;
   //             tex_filename = tex_filename.parent_path() / aiPath.C_Str();
   //             TextureImporter::Get().CreateTexture(tex_filename.string());
   //             material->SetTexture("$Normal", tex_filename.string());
   //         }
   //     }
    //    Logger::Debug(name, "导入模型成功%s", std::string{ filename }.c_str());
    //    return &model;
    //}

	

    return nullptr;
}

const Model* MeshImporter::CreateFromGeometry(std::string_view name, const Geometry::MeshData& data)
{
    XID modelID = StringToID(name);
    if (m_Models.count(modelID))
        return nullptr;

    auto& model = m_Models[modelID];
    model.meshDatas.resize(1);
    model.materials.resize(1);
    model.meshDatas[0].m_pTexcoordArrays.resize(1);
    model.meshDatas[0].m_IndexCount = (uint32_t)(data.indices32.size());
    model.meshDatas[0].m_MaterialIndex = 0;
    CD3D11_BUFFER_DESC bufferDesc(0, D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };

    initData.pSysMem = data.vertices.data();
    bufferDesc.ByteWidth = (uint32_t)data.vertices.size() * sizeof(XMFLOAT3);
    m_pDevice->CreateBuffer(&bufferDesc, &initData, model.meshDatas[0].m_pVertices.ReleaseAndGetAddressOf());

    initData.pSysMem = data.normals.data();
    bufferDesc.ByteWidth = (uint32_t)data.normals.size() * sizeof(XMFLOAT3);
    m_pDevice->CreateBuffer(&bufferDesc, &initData, model.meshDatas[0].m_pNormals.ReleaseAndGetAddressOf());

    initData.pSysMem = data.texcoords.data();
    bufferDesc.ByteWidth = (uint32_t)data.texcoords.size() * sizeof(XMFLOAT2);
    m_pDevice->CreateBuffer(&bufferDesc, &initData, model.meshDatas[0].m_pTexcoordArrays[0].ReleaseAndGetAddressOf());

    initData.pSysMem = data.tangents.data();
    bufferDesc.ByteWidth = (uint32_t)data.tangents.size() * sizeof(XMFLOAT4);
    m_pDevice->CreateBuffer(&bufferDesc, &initData, model.meshDatas[0].m_pTangents.ReleaseAndGetAddressOf());


        initData.pSysMem = data.indices32.data();
        bufferDesc = CD3D11_BUFFER_DESC((uint32_t)data.indices32.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
        m_pDevice->CreateBuffer(&bufferDesc, &initData, model.meshDatas[0].m_pIndices.ReleaseAndGetAddressOf());


    return &model;
}


const Model* MeshImporter::GetModel(std::string_view name) const
{
    XID nameID = StringToID(name);
    if (auto it = m_Models.find(nameID); it != m_Models.end())
        return &it->second;
    return nullptr;
}

Model* MeshImporter::GetModel(std::string_view name)
{
    XID nameID = StringToID(name);
    if (m_Models.count(nameID))
        return &m_Models[nameID];
    return nullptr;
}

const Model* MeshImporter::GetModel(XID nameID) const
{
	if (auto it = m_Models.find(nameID); it != m_Models.end())
		return &it->second;
	return nullptr;
}

Model* MeshImporter::GetModel(XID nameID)
{
	if (m_Models.count(nameID))
		return &m_Models[nameID];
	return nullptr;
}

