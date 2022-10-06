
#ifndef CASCADED_SHADOW_HLSL
#define CASCADED_SHADOW_HLSL

#include "ConstantBuffers.hlsl"

// ʹ��ƫ������shadow map�е�texelsӳ�䵽������Ⱦ��ͼԪ�Ĺ۲�ռ�ƽ����
// ����Ƚ������ڱȽϲ�������Ӱ����
// ������ǿ�������ģ��Ҽٶ�������ƽ��϶��ʱ�����Ч
#ifndef USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG
#define USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG 0
#endif

// �����ڲ�ͬ����֮�����Ӱֵ��ϡ���shadow maps�Ƚ�С
// ��artifacts����������֮��ɼ���ʱ����Ϊ��Ч
#ifndef BLEND_BETWEEN_CASCADE_LAYERS_FLAG
#define BLEND_BETWEEN_CASCADE_LAYERS_FLAG 0
#endif

// �����ַ���Ϊ��ǰ����ƬԪѡ����ʵļ�����
// Interval-based Selection ����׶�����ȷ���������ƬԪ����Ƚ��бȽ�
// Map-based Selection �ҵ�����������shadow map��Χ�е���С����
#ifndef SELECT_CASCADE_BY_INTERVAL_FLAG
#define SELECT_CASCADE_BY_INTERVAL_FLAG 0
#endif

// ������Ŀ
#ifndef CASCADE_COUNT_FLAG
#define CASCADE_COUNT_FLAG 3
#endif

// ���������£�ʹ��3-4��������������BLEND_BETWEEN_CASCADE_LAYERS_FLAG��
// ���������ڵͶ�PC���߶�PC���Դ���������Ӱ���Լ�����Ļ�ϵش�
// ��ʹ�ø����PCF��ʱ�����Ը��߶�PCʹ�û���ƫ�������ƫ��

Texture2DArray g_TextureShadow : register(t10);
SamplerComparisonState g_SamplerShadow : register(s10);

static const float4 s_CascadeColorsMultiplier[8] =
{
    float4(1.5f, 0.0f, 0.0f, 1.0f),
    float4(0.0f, 1.5f, 0.0f, 1.0f),
    float4(0.0f, 0.0f, 5.5f, 1.0f),
    float4(1.5f, 0.0f, 5.5f, 1.0f),
    float4(1.5f, 1.5f, 0.0f, 1.0f),
    float4(1.0f, 1.0f, 1.0f, 1.0f),
    float4(0.0f, 1.0f, 5.5f, 1.0f),
    float4(0.5f, 3.5f, 0.75f, 1.0f)
};

//--------------------------------------------------------------------------------------
// Ϊ��Ӱ�ռ��texels�����Ӧ���տռ�
//--------------------------------------------------------------------------------------
void CalculateRightAndUpTexelDepthDeltas(float3 shadowTexDDX, float3 shadowTexDDY,
                                         out float upTextDepthWeight,
                                         out float rightTextDepthWeight)
{
    // ����ʹ��X��Y�е�ƫ����������任����������Ҫ��������Ǵ���Ӱ�ռ�任����Ļ�ռ䣬
    // ��Ϊ��Щ�����������Ǵ���Ļ�ռ�任����Ӱ�ռ䡣�µľ����������Ǵ���Ӱͼ��texelsӳ��
    // ����Ļ�ռ䡣�⽫��������Ѱ�Ҷ�Ӧ������ص���Ļ�ռ���ȡ�
    // �ⲻ��һ�������Ľ����������Ϊ���ٶ������еļ�������һ��ƽ�档
    // [TODO]һ�ָ�׼ȷ��Ѱ��ʵ����ȵķ���Ϊ�������ӳ���Ⱦ������shadow map��
    
    // �ڴ��������£�ʹ��ƫ�ƻ򷽲���Ӱ��ͼ��һ�ָ��õġ��ܹ�����αӰ�ķ���
    
    float2x2 matScreenToShadow = float2x2(shadowTexDDX.xy, shadowTexDDY.xy);
    float det = determinant(matScreenToShadow);
    float invDet = 1.0f / det;
    float2x2 matShadowToScreen = float2x2(
        matScreenToShadow._22 * invDet, matScreenToShadow._12 * -invDet,
        matScreenToShadow._21 * -invDet, matScreenToShadow._11 * invDet);
    
    float2 rightShadowTexelLocation = float2(g_TexelSize, 0.0f);
    float2 upShadowTexelLocation = float2(0.0f, g_TexelSize);
    
    // ͨ����Ӱ�ռ䵽��Ļ�ռ�ľ���任�ұߵ�texel
    float2 rightTexelDepthRatio = mul(rightShadowTexelLocation, matShadowToScreen);
    float2 upTexelDepthRatio = mul(upShadowTexelLocation, matShadowToScreen);
    
    // �������ڿ��Լ�����shadow map���Һ������ƶ�ʱ����ȵı仯ֵ
    // ����ʹ��x�����y����任�ı�ֵ������Ļ�ռ�X��Y��ȵĵ���������仯ֵ
    upTextDepthWeight =
        upTexelDepthRatio.x * shadowTexDDX.z 
        + upTexelDepthRatio.y * shadowTexDDY.z;
    rightTextDepthWeight =
        rightTexelDepthRatio.x * shadowTexDDX.z 
        + rightTexelDepthRatio.y * shadowTexDDY.z;
}

//--------------------------------------------------------------------------------------
// ʹ��PCF�������ͼ��������ɫ�ٷֱ�
//--------------------------------------------------------------------------------------
float CalculatePCFPercentLit(int currentCascadeIndex,
                             float4 shadowTexCoord, 
                             float rightTexelDepthDelta, 
                             float upTexelDepthDelta,
                             float blurSize)
{
    float percentLit = 0.0f;
    // ��ѭ������չ�����������PCF��С�ǹ̶��Ļ�������ʹ������ʱƫ�ƴӶ���������
    for (int x = g_PCFBlurForLoopStart; x < g_PCFBlurForLoopEnd; ++x)
    {
        for (int y = g_PCFBlurForLoopStart; y < g_PCFBlurForLoopEnd; ++y)
        {
            float depthCmp = shadowTexCoord.z;
            // һ���ǳ��򵥵Ľ��PCF���ƫ������ķ�����ʹ��һ��ƫ��ֵ
            // ���ҵ��ǣ������ƫ�ƻᵼ��Peter-panning����Ӱ�ܳ����壩
            // ��С��ƫ���ֻᵼ����Ӱʧ��
            depthCmp -= g_ShadowBias;
            if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
            {
                depthCmp += rightTexelDepthDelta * (float)x + upTexelDepthDelta * (float)y;
            }
            // ���任����������ͬ��Ӱͼ�е���Ƚ��бȽ�
            percentLit += g_TextureShadow.SampleCmpLevelZero(g_SamplerShadow,
                float3(
                    shadowTexCoord.x + (float)x * g_TexelSize,
                    shadowTexCoord.y + (float)y * g_TexelSize,
                    (float)currentCascadeIndex
                ),
                depthCmp);
        }
    }
    percentLit /= blurSize;
    return percentLit;
}

//--------------------------------------------------------------------------------------
// ������������֮��Ļ���� �� ��Ͻ��ᷢ��������
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForInterval(int currentCascadeIndex,
                                     inout float pixelDepth,
                                     inout float currentPixelsBlendBandLocation,
                                     out float blendBetweenCascadesAmount)
{
    
    //                  pixelDepth
    //           |<-      ->|
    // /-+-------/----------+------/--------
    // 0 N     F[0]               F[i]
    //           |<-blendInterval->|
    // blendBandLocation = 1 - depth/F[0] or
    // blendBandLocation = 1 - (depth-F[0]) / (F[i]-F[0])
    // blendBandLocationλ��[0, g_CascadeBlendArea]ʱ������[0, 1]�Ĺ���
    
    // ������Ҫ���㵱ǰshadow map�ı�Ե�ش��������ｫ�ᵭ������һ������
    // Ȼ�����ǾͿ�����ǰ���뿪�������PCF forѭ��
    float blendInterval = g_CascadeFrustumsEyeSpaceDepthsFloat4[currentCascadeIndex].x;
    
    // ��ԭ��Ŀ���ⲿ�ִ������������
    if (currentCascadeIndex > 0)
    {
        int blendIntervalbelowIndex = currentCascadeIndex - 1;
        pixelDepth -= g_CascadeFrustumsEyeSpaceDepthsFloat4[blendIntervalbelowIndex].x;
        blendInterval -= g_CascadeFrustumsEyeSpaceDepthsFloat4[blendIntervalbelowIndex].x;
    }
    
    // ��ǰ���صĻ�ϵش���λ��
    currentPixelsBlendBandLocation = 1.0f - pixelDepth / blendInterval;
    // blendBetweenCascadesAmount�������յ���Ӱɫ��ֵ
    blendBetweenCascadesAmount = currentPixelsBlendBandLocation / g_CascadeBlendArea;
}

//--------------------------------------------------------------------------------------
// ������������֮��Ļ���� �� ��Ͻ��ᷢ��������
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForMap(float4 shadowMapTexCoord,
                                inout float currentPixelsBlendBandLocation,
                                inout float blendBetweenCascadesAmount)
{
    //   _____________________
    //  |       map[i+1]      |
    //  |                     |
    //  |      0_______0      |
    //  |______| map[i]|______|
    //         |  0.5  |
    //         |_______|
    //         0       0
    // blendBandLocation = min(tx, ty, 1-tx, 1-ty);
    // blendBandLocationλ��[0, g_CascadeBlendArea]ʱ������[0, 1]�Ĺ���
    float2 distanceToOne = float2(1.0f - shadowMapTexCoord.x, 1.0f - shadowMapTexCoord.y);
    currentPixelsBlendBandLocation = min(shadowMapTexCoord.x, shadowMapTexCoord.y);
    float currentPixelsBlendBandLocation2 = min(distanceToOne.x, distanceToOne.y);
    currentPixelsBlendBandLocation =
        min(currentPixelsBlendBandLocation, currentPixelsBlendBandLocation2);
    
    blendBetweenCascadesAmount = currentPixelsBlendBandLocation / g_CascadeBlendArea;
}

//--------------------------------------------------------------------------------------
// ���㼶����ʾɫ���߶�Ӧ�Ĺ���ɫ
//--------------------------------------------------------------------------------------
float4 GetCascadeColorMultipler(int currentCascadeIndex, 
                                int nextCascadeIndex, 
                                float blendBetweenCascadesAmount)
{
    return lerp(s_CascadeColorsMultiplier[nextCascadeIndex], 
                s_CascadeColorsMultiplier[currentCascadeIndex], 
                blendBetweenCascadesAmount);
}

//--------------------------------------------------------------------------------------
// ���㼶����Ӱ
//--------------------------------------------------------------------------------------
float CalculateCascadedShadow(float4 shadowMapTexCoordViewSpace, 
                              float currentPixelDepth,
                              out int currentCascadeIndex,
                              out int nextCascadeIndex,
                              out float blendBetweenCascadesAmount)
{
    float4 shadowMapTexCoord = 0.0f;
    float4 shadowMapTexCoord_blend = 0.0f;
    
    float4 visualizeCascadeColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float percentLit = 0.0f;
    float percentLit_blend = 0.0f;

    float upTextDepthWeight = 0;
    float rightTextDepthWeight = 0;
    float upTextDepthWeight_blend = 0;
    float rightTextDepthWeight_blend = 0;

    float blurSize = g_PCFBlurForLoopEnd - g_PCFBlurForLoopStart;
    blurSize *= blurSize;
         
    int cascadeFound = 0;
    nextCascadeIndex = 1;
    
    //
    // ȷ���������任��Ӱ��������
    //
    
    // ����׶���Ǿ��Ȼ��� �� ʹ����Interval-Based Selection����ʱ
    // ���Բ���Ҫѭ��������
    // �����������currentPixelDepth����������ȷ����׶�������в���
    // Interval-Based Selection
    if (SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        currentCascadeIndex = 0;
        //                               Depth
        // /-+-------/----------------/----+-------/----------/
        // 0 N     F[0]     ...      F[i]        F[i+1] ...   F
        // Depth > F[i] to F[0] => index = i+1
        if (CASCADE_COUNT_FLAG > 1)
        {
            float4 currentPixelDepthVec = currentPixelDepth;
            float4 cmpVec1 = (currentPixelDepthVec > g_CascadeFrustumsEyeSpaceDepthsFloat[0]);
            float4 cmpVec2 = (currentPixelDepthVec > g_CascadeFrustumsEyeSpaceDepthsFloat[1]);
            float index = dot(float4(CASCADE_COUNT_FLAG > 0,
                                     CASCADE_COUNT_FLAG > 1,
                                     CASCADE_COUNT_FLAG > 2,
                                     CASCADE_COUNT_FLAG > 3),
                              cmpVec1) +
                          dot(float4(CASCADE_COUNT_FLAG > 4,
                                     CASCADE_COUNT_FLAG > 5,
                                     CASCADE_COUNT_FLAG > 6,
                                     CASCADE_COUNT_FLAG > 7),
                              cmpVec2);
            index = min(index, CASCADE_COUNT_FLAG - 1);
            currentCascadeIndex = (int) index;
        }
        
        shadowMapTexCoord = shadowMapTexCoordViewSpace * g_CascadeScale[currentCascadeIndex] + g_CascadeOffset[currentCascadeIndex];
    }

    // Map-Based Selection
    if ( !SELECT_CASCADE_BY_INTERVAL_FLAG )
    {
        currentCascadeIndex = 0;
        if (CASCADE_COUNT_FLAG == 1)
        {
            shadowMapTexCoord = shadowMapTexCoordViewSpace * g_CascadeScale[0] + g_CascadeOffset[0];
        }
        if (CASCADE_COUNT_FLAG > 1)
        {
            // Ѱ������ļ�����ʹ����������λ������߽���
            // minBorder < tx, ty < maxBorder
            for (int cascadeIndex = 0; cascadeIndex < CASCADE_COUNT_FLAG && cascadeFound == 0; ++cascadeIndex)
            {
                shadowMapTexCoord = shadowMapTexCoordViewSpace * g_CascadeScale[cascadeIndex] + g_CascadeOffset[cascadeIndex];
                if (min(shadowMapTexCoord.x, shadowMapTexCoord.y) > g_MinBorderPadding
                    && max(shadowMapTexCoord.x, shadowMapTexCoord.y) < g_MaxBorderPadding)
                {
                    currentCascadeIndex = cascadeIndex;
                    cascadeFound = 1;
                }
            }
        }
    }
    
    //
    // ���㵱ǰ������PCF
    // 
    float3 shadowMapTexCoordDDX;
    float3 shadowMapTexCoordDDY;
    // ��Щƫ�����ڼ���ͶӰ����ռ�����texel��Ӧ�����տռ䲻ͬ�����������ȱ仯
    if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
    {
        // ������տռ��ƫ��ӳ�䵽ͶӰ����ռ�ı仯��
        shadowMapTexCoordDDX = ddx(shadowMapTexCoordViewSpace);
        shadowMapTexCoordDDY = ddy(shadowMapTexCoordViewSpace);
        
        shadowMapTexCoordDDX *= g_CascadeScale[currentCascadeIndex];
        shadowMapTexCoordDDY *= g_CascadeScale[currentCascadeIndex];
        
        CalculateRightAndUpTexelDepthDeltas(shadowMapTexCoordDDX, shadowMapTexCoordDDY,
                                            upTextDepthWeight, rightTextDepthWeight);
    }
    
    visualizeCascadeColor = s_CascadeColorsMultiplier[currentCascadeIndex];
    
    percentLit = CalculatePCFPercentLit(currentCascadeIndex, shadowMapTexCoord,
                                        rightTextDepthWeight, upTextDepthWeight, blurSize);
    
    
    //
    // ����������֮����л��
    //
    if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG)
    {
        // Ϊ��һ�������ظ�����ͶӰ��������ļ���
        // ��һ������������������������֮��ģ��
        nextCascadeIndex = min(CASCADE_COUNT_FLAG - 1, currentCascadeIndex + 1);
    }
    
    blendBetweenCascadesAmount = 1.0f;
    float currentPixelsBlendBandLocation = 1.0f;
    if (SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1)
        {
            CalculateBlendAmountForInterval(currentCascadeIndex, currentPixelDepth,
                currentPixelsBlendBandLocation, blendBetweenCascadesAmount);
        }
    }
    else
    {
        if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG)
        {
            CalculateBlendAmountForMap(shadowMapTexCoord,
                currentPixelsBlendBandLocation, blendBetweenCascadesAmount);
        }
    }
    
    if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1)
    {
        if (currentPixelsBlendBandLocation < g_CascadeBlendArea)
        {
            // ������һ������ͶӰ��������
            shadowMapTexCoord_blend = shadowMapTexCoordViewSpace * g_CascadeScale[nextCascadeIndex] + g_CascadeOffset[nextCascadeIndex];
            
            // �ڼ���֮����ʱ��Ϊ��һ����Ҳ���м���
            if (currentPixelsBlendBandLocation < g_CascadeBlendArea)
            {
                // ��ǰ�����ڻ�ϵش���
                if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
                {
                    CalculateRightAndUpTexelDepthDeltas(shadowMapTexCoordDDX, shadowMapTexCoordDDY,
                                                        upTextDepthWeight_blend, rightTextDepthWeight_blend);
                }
                percentLit_blend = CalculatePCFPercentLit(nextCascadeIndex, shadowMapTexCoord_blend,
                                                          rightTextDepthWeight_blend, upTextDepthWeight_blend, blurSize);
                // ������������PCF���
                percentLit = lerp(percentLit_blend, percentLit, blendBetweenCascadesAmount);
            }
        }
    }

    return percentLit;
}


#endif // CASCADED_SHADOW_HLSL
