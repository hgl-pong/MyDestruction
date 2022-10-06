#ifndef SKYBOX_H
#define SKYBOX_H

#include "Geometry.h"
#include "Material.h"
#include "MeshData.h"
#include "Transform.h"
#include "IEffect.h"

struct Model;
namespace Graphics {
    class SkyBox
    {
    public:
        template <class T>
        using ComPtr = Microsoft::WRL::ComPtr<T>;

        SkyBox() = default;
        ~SkyBox() = default;

        SkyBox(const SkyBox&) = default;
        SkyBox& operator=(const SkyBox&) = default;

        SkyBox(SkyBox&&) = default;
        SkyBox& operator=(SkyBox&&) = default;

        // 获取物体变换
        Transform& GetTransform();
        // 获取物体变换
        const Transform& GetTransform() const;

        //
        // 模型
        //
        void SetModel(const Model* pModel);

        //
        // 绘制
        //

        // 绘制对象
        void Draw(ID3D11DeviceContext* deviceContext, IEffect* effect);

    private:
        const Model* m_pModel = nullptr;
        Transform m_Transform = {};
    };

}


#endif
