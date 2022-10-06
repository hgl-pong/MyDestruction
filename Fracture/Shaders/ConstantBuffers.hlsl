
#ifndef CONSTANTBUFFERS_HLSL
#define CONSTANTBUFFERS_HLSL

cbuffer CBChangesEveryInstanceDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    matrix g_WorldView;
    matrix g_WorldViewProj;
}

cbuffer CBCascadedShadow : register(b1)
{
    matrix g_ShadowView;            
    float4 g_CascadeOffset[8];      // ShadowPT�����ƽ����
    float4 g_CascadeScale[8];       // ShadowPT�����������
    int    g_VisualizeCascades;     // 1ʹ�ò�ͬ����ɫ���ӻ�������Ӱ��0���Ƴ���
    int    g_PCFBlurForLoopStart;   // ѭ����ʼֵ��5x5��PCF�˴�-2��ʼ
    int    g_PCFBlurForLoopEnd;     // ѭ������ֵ��5x5��PCF��Ӧ����Ϊ3
    int    g_Pad;
    
    
    // ��Map-based Selection�������⽫������Ч��Χ�ڵ����ء�
    // ��û�б߽�ʱ��Min��Max�ֱ�Ϊ0��1
    float  g_MinBorderPadding;      // (kernelSize / 2) / (float)shadowSize
    float  g_MaxBorderPadding;      // 1.0f - (kernelSize / 2) / (float)shadowSize
    float  g_ShadowBias;            // ������ӰαӰ��ƫ��ֵ����ЩαӰ����ΪPCF���Ӿ�
    float  g_CascadeBlendArea;      // ����֮���ص���ʱ�Ļ������
    
    
    float  g_TexelSize;             // Shadow map�����ش�С
    float3 g_LightDir;              // ��Դ����
    
    float4 g_CascadeFrustumsEyeSpaceDepthsFloat[2]; // ��ͬ����׶��Զƽ���Zֵ���������ֿ�
    float4 g_CascadeFrustumsEyeSpaceDepthsFloat4[8];// ��float4���Ѷ���ռ����ʽʹ�ÿ������������yzw��������
}


#endif // CONSTANTBUFFERS_HLSL
