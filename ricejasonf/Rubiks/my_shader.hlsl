#define OBJECTSHADER_LAYOUT_COMMON
#define TRANSPARENT
#include "objectHF.hlsli"

float my_hash(float2 p) {
  p = frac(p * float2(234.34, 435.345));
  p += dot(p, p + 34.23);
  return frac(p.x * p.y);
}

float4 big_lighting(PixelInput input, float4 color_, float3 normal)
{
	const float depth = input.pos.z;
	const float lineardepth = input.pos.w;
	const uint2 pixel = input.pos.xy; // no longer pixel center!
	const float2 ScreenCoord = input.pos.xy * GetCamera().internal_resolution_rcp; // use pixel center!

	float4 uvsets = input.GetUVSets();

	write_mipmap_feedback(push.materialIndex, ddx_coarse(uvsets), ddy_coarse(uvsets));

	ShaderMeshInstance meshinstance = load_instance(input.GetInstanceIndex());

	Surface surface;
	surface.init();

	surface.N = normalize(normal);
	surface.occlusion = input.ao;

	surface.P = input.pos3D;
	surface.V = GetCamera().position - surface.P;
	float dist = length(surface.V);
	surface.V /= dist;

	surface.T = input.tan;
	surface.T.w = surface.T.w < 0 ? -1 : 1;
	float3 bitangent = cross(surface.T.xyz, normal) * surface.T.w;
	float3x3 TBN = float3x3(surface.T.xyz, bitangent, normal); // unnormalized TBN! http://www.mikktspace.com/
	
	surface.T.xyz = normalize(surface.T.xyz);

	surface.baseColor *= color_;

	surface.layerMask = GetMaterial().layerMask & meshinstance.layerMask;


	float4 surfaceMap = 1;

	// Emissive map:
	surface.emissiveColor = GetMaterial().GetEmissive();
	surface.emissiveColor *= Unpack_R11G11B10_FLOAT(meshinstance.emissive);

	[branch]
	if (!GetMaterial().IsUsingSpecularGlossinessWorkflow())
	{
		// Premultiply these before evaluating decals:
		surfaceMap.g *= GetMaterial().roughness;
		surfaceMap.b *= GetMaterial().metalness;
		surfaceMap.a *= GetMaterial().reflectance;
	}

	const uint flat_tile_index = GetFlatTileIndex(pixel);

	[branch]
	if (any(surface.bumpColor))
	{
		surface.N = normalize(mul(surface.bumpColor, TBN));
	}

	float4 specularMap = 1;

	surface.create(GetMaterial(), surface.baseColor, surfaceMap, specularMap);
	
#if 0
	[branch]
	if (GetCamera().texture_ao_index >= 0)
	{
		surface.occlusion *= bindless_textures_float[GetCamera().texture_ao_index].SampleLevel(sampler_linear_clamp, ScreenCoord, 0).r;
	}
#endif

	surface.sss = GetMaterial().subsurfaceScattering;
	surface.sss_inv = GetMaterial().subsurfaceScattering_inv;

	surface.pixel = pixel;
	surface.screenUV = ScreenCoord;

	surface.update();

	float3 ambient = GetAmbient(surface.N);
	ambient = lerp(ambient, ambient * surface.sss.rgb, saturate(surface.sss.a));

	Lighting lighting;
	lighting.create(0, 0, ambient, 0);

	
	float4 color = surface.baseColor;

	LightMapping(meshinstance.lightmap, input.atl, lighting, surface);

	TiledLighting(surface, lighting, flat_tile_index);

	[branch]
	if (GetCamera().texture_ssr_index >= 0)
	{
		float4 ssr = bindless_textures[GetCamera().texture_ssr_index].SampleLevel(sampler_linear_clamp, ScreenCoord, 0);
		lighting.indirect.specular = lerp(lighting.indirect.specular, ssr.rgb * surface.F, ssr.a);
	}
	[branch]
	if (GetCamera().texture_ssgi_index >= 0)
	{
		surface.ssgi = bindless_textures[GetCamera().texture_ssgi_index].SampleLevel(sampler_linear_clamp, ScreenCoord, 0).rgb;
	}

	ApplyLighting(surface, lighting, color);

	ApplyFog(dist, surface.V, color);

	color = clamp(color, 0, 65000);

	// end point:
  return color;
}

[earlydepthstencil]
float4 main(PixelInput input) : SV_TARGET
{
  float time = GetFrame().time;
	float4 uvsets = input.GetUVSets();
	float2 uv = uvsets.xy;

  float4 color = 0;

  float thresh = 0.01;
  float x = uv.x;
  float y = uv.y;
  float z = 1;

  // Golden ratio
  float golden_phi = 1.618033;
  float N = 5.0;
  float2 a;
  a.x = uv.y * N + uv.x;
  a.y = uv.y * N - uv.x;
  float N_rcp = 1 / N / 2;
  a = smoothstep(thresh, -thresh, abs(fmod(a, N_rcp)) - .01);
  float mask = smoothstep(0, 1.0 + thresh, abs(a.x + a.y));
  #if 0
  float target_theta = fmod(n, golden_phi);
  float target_phi = asin(2 * n / (2 * N + 1));
  float2 target = float2(target_theta, target_phi);
  float phi = acos(z);
  float theta = tan(y / x);
  float2 current = float2(phi, theta);
  float mask = smoothstep(0, thresh, distance(current, target));
  #endif
  color.rg += a;
  color.b += mask;

  /*
      for i in range(samples):
        y = 1 - (i / float(samples - 1)) * 2  # y goes from 1 to -1
        radius = math.sqrt(1 - y * y)  # radius at y

        theta = phi * i  # golden angle increment

        x = math.cos(theta) * radius
        z = math.sin(theta) * radius

        points.append((x, y, z))
  */

  //color = big_lighting(input, color, input.nor);


	return float4(color.rgb, 1.0);
}
