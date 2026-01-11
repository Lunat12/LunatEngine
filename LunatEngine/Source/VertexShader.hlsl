cbuffer Transforms : register(b0)
{
    float4x4 mvp; // mvp = model * view * projection
};

struct vertexShader
{
    float2 coord : TEXCOORD;
    float4 pos : SV_POSITION;
};

vertexShader main(float3 pos : MY_POSITION, float2 coord : TEXCOORD)
{
    vertexShader output;
    output.pos = mul(float4(pos, 1.0), mvp); // Pre-multiply vector by MVP matrix
    output.coord = coord;
    return output;

}