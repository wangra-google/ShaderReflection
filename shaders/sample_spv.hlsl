Texture2D     MyTexture : register(t0, space0); // D3D_SIT_TEXTURE2D
SamplerState  MySampler : register(s1, space1); // D3D_SIT_SAMPLER

struct RGB {
  float r;
  float g;
  float b;
};

struct UBO {
  float4x4  XformMatrix;
  float3    Scale;
  RGB       Rgb;
  float     t;
  float2    uv; 
};

ConstantBuffer<UBO> MyConstants[2] : register(b2, space2); // D3D_SIT_CBUFFER

struct Data {
  float3  Element_f3;
  float2  Element_f2;
};

ConsumeStructuredBuffer<Data> MyBufferIn : register(u3, space2); // D3D_SIT_UAV_CONSUME_STRUCTURED
AppendStructuredBuffer<Data> MyBufferOut : register(u4, space2); // D3D_SIT_UAV_APPEND_STRUCTURED

struct PSInput {
  float4  Position  : SV_POSITION;
  float3  Normal    : NORMAL;
  float3  Color     : COLOR;
  float   Alpha     : OPACITY;
  float4  Scaling   : SCALE;
  float2  TexCoord0 : TEXCOORD0;
  float2  TexCoord1 : TEXCOORD1;
  float2  TexCoord2 : TEXCOORD2;
};

struct PSOutput {
  float4  oColor0 : SV_TARGET0;
  float4  oColor1 : SV_TARGET1;
  float4  oColor2 : SV_TARGET2;
  float4  oColor3 : SV_TARGET3;
  float4  oColor4 : SV_TARGET4;
  float4  oColor5 : SV_TARGET5;
  float4  oColor6 : SV_TARGET6;
  float4  oColor7 : SV_TARGET7;
};

struct myStruct
{
    float3 f3;
	float f1;
	float2 f2;
};

TextureBuffer<myStruct> texture_buffer; // D3D_SIT_TBUFFER
RWTexture2D<float2>     rw_texture[2]; // D3D_SIT_UAV_RWTYPED
StructuredBuffer<myStruct> sturctured_buffer; // D3D_SIT_STRUCTURED
RWStructuredBuffer<myStruct> rw_sturctured_buffer; // D3D_SIT_UAV_RWSTRUCTURED
ByteAddressBuffer ba_buffer; // D3D_SIT_BYTEADDRESS
RWByteAddressBuffer rw_ba_buffer; // D3D_SIT_RWBYTEADDRESS
RWTexture2DArray<float2>     rw_texture_array; // D3D_SIT_UAV_RWTYPED
Texture2DMS<float4, 128> ms_texture;
Texture2DMSArray<float4, 64> ms_texture_array;
cbuffer c_buffer : register(b11, space2) { int2 cbuffer_i2; float3 cbuffer_f3; }
tbuffer t_buffer : register(t15)
{
	float3 tbuffer_f3;
	uint tbuffer_u1;
	int2 tbuffer_i2;
};

PSOutput main(PSInput input)
{
  Data val = MyBufferIn.Consume();
  MyBufferOut.Append(val);
  
  rw_texture[0][uint2(0,0)] = 1;
  rw_sturctured_buffer[0].f3.x = 1;
  rw_ba_buffer.Store(1, 1);
  int w,h,n;
  ms_texture.GetDimensions(w,h,n); 
  int w1,h1,e1,n1;
  ms_texture_array.GetDimensions(w1,h1,e1,n1); 
  e1 *= texture_buffer.f3.x;

  PSOutput ret;
  ret.oColor0 = mul(MyConstants[0].XformMatrix, input.Position) * n * e1 * cbuffer_i2.x * tbuffer_f3.x;
  ret.oColor1 = float4(input.Normal, 1) + float4(MyConstants[0].Scale, 0);
  ret.oColor2 = float4(input.Color, 1);
  ret.oColor3 = float4(MyTexture.Sample(MySampler, input.TexCoord0).xyz, input.Alpha);
  ret.oColor4 = input.Scaling * sturctured_buffer[0].f3.x * ba_buffer.Load(0);
  ret.oColor5 = float4(input.TexCoord0, 0, 0);
  ret.oColor6 = float4(input.TexCoord1, 0, 0);
  ret.oColor7 = float4(input.TexCoord2, 0, 0);
  return ret;
}