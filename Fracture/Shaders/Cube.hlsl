#ifndef CUBE_HLSL
#define CUBE_HLSL
// cbuffer ConstantBuffer : register(b0)
// {
//     matrix g_World; // matrix������float4x4���������row_major������£�����Ĭ��Ϊ��������
//     matrix g_View;  // ������ǰ�����row_major��ʾ��������
//     matrix g_Proj;  // �ý̳�����ʹ��Ĭ�ϵ��������󣬵���Ҫ��C++�����Ԥ�Ƚ��������ת�á�
// }

uniform matrix g_MViewProj;
struct VertexIn
{
    float3 posL : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};

VertexOut CubeVS(VertexIn vIn)
{
    VertexOut vOut;
    //vOut.posH = mul(float4(vIn.posL, 1.0f), g_World);  // mul ���Ǿ���˷�, �����*Ҫ���������Ϊ
    //vOut.posH = mul(vOut.posH, g_View);               // ��������ȵ��������󣬽��Ϊ
    vOut.posH = mul(float4(vIn.posL, 1.0f), g_MViewProj);               // Cij = Aij * Bij
    vOut.color = vIn.color;                         // ����alphaͨ����ֵĬ��Ϊ1.0
    return vOut;
}

// ������ɫ��
float4 CubePS(VertexOut pIn) : SV_Target
{
    return pIn.color;
}

#endif