#define OBJECTSHADER_LAYOUT_COMMON
#include "objectHF.hlsli"

[earlydepthstencil]
float4 main(PixelInput input) : SV_TARGET
{
  float4 color;
  color.rgba = 1.0;
  color.b = 0.0;

	return color;
}
