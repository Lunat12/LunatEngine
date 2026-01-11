cbuffer Material : register(b1)
{
    float4 color;
    bool hasColorTex;
}


Texture2D colourTex : register(t0);
SamplerState colourSampler : register(s0);

float4 main(float2 coords : TEXCOORD) : SV_TARGET
{
    return hasColorTex ? colourTex.Sample(colourSampler, coords) * color : color;

}