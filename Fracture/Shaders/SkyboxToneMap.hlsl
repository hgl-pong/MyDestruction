
#ifndef SKYBOX_TONE_MAP_HLSL
#define SKYBOX_TONE_MAP_HLSL

#ifndef MSAA_SAMPLES
#define MSAA_SAMPLES 1
#endif

uniform matrix g_ViewProj;

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 texCoord : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// ����, ��պе�
// ʹ����պм�������Ⱦ
//--------------------------------------------------------------------------------------
TextureCube<float4> g_SkyboxTexture : register(t5);
Texture2DMS<float, MSAA_SAMPLES> g_DepthTexture : register(t6);

// ������ز����ĳ�����Ⱦ������
Texture2DMS<float4, MSAA_SAMPLES> g_LitTexture : register(t7);

struct SkyboxVSOut
{
    float4 posViewport : SV_Position;
    float3 skyboxCoord : skyboxCoord;
};

SkyboxVSOut SkyboxVS(VertexPosNormalTex input)
{
    SkyboxVSOut output;
    
    // ע�⣺��Ҫ�ƶ���պв�ȷ�����ֵΪ1(����ü�)
    output.posViewport = mul(float4(input.posL, 0.0f), g_ViewProj).xyww;
    output.skyboxCoord = input.posL;
    
    return output;
}

SamplerState g_SamplerDiffuse : register(s0);

float4 SkyboxPS(SkyboxVSOut input) : SV_Target
{
    uint2 coords = input.posViewport.xy;

    float3 lit = float3(0.0f, 0.0f, 0.0f);
    float skyboxSamples = 0.0f;
#if MSAA_SAMPLES <= 1
    [unroll]
#endif
    for (unsigned int sampleIndex = 0; sampleIndex < MSAA_SAMPLES; ++sampleIndex)
    {
        float depth = g_DepthTexture.Load(coords, sampleIndex);

        // ע�⣺����Z
        if (depth <= 0.0f)
        {
            ++skyboxSamples;
        }

        else
        {
            lit += g_LitTexture.Load(coords, sampleIndex).xyz;
        }
    }

    // �������û�г�����Ⱦ������Ⱦ��պ�
    [branch]
    if (skyboxSamples > 0)
    {
        float3 skybox = g_SkyboxTexture.Sample(g_SamplerDiffuse, input.skyboxCoord).xyz;
        lit += skyboxSamples * skybox;
    }
    
    // Resolve ���ز���(�򵥺����˲�)
    return float4(lit * rcp((float) MSAA_SAMPLES), 1.0f);
}


#endif // SKYBOX_TONE_MAP_HLSL
