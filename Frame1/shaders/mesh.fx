
#define FLOAT_ZERO 1e-6
#define FLOAT_PI 3.141592653

struct OMaterial
{
	float4 ambient;
	float4 emissive;
	float4 specular;
	float4 diffuse;
	float shinies;
};

struct OLight
{
	float4 ambient;
	float4 color;
	float4 direction;
};

struct VS_INPUT
{
	float4 position : POSITION;
	float4 normal 	: NORMAL;
	float2 texco	: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position	: POSITION;
	float4 color	: COLOR;
	float2 texco	: TEXCOORD;
};

struct PS_INPUT
{
	float2 texco	: TEXCOORD0;
	float4 color	: COLOR;
};

struct PS_OUTPUT
{
	float4 color : COLOR0;
};

struct VS_INPUT_LEAF
{
	float4 center	:	POSITION;
	float4 normal	:	NORMAL;
	float2 texco 	:	TEXCOORD0;
	
	float2 para1 	:	TEXCOORD1;	// alpha_, scalar_
	float2 para2	:	TEXCOORD2;	// ovx_, ovy_
	float2 para3	:	TEXCOORD3;	// ovz_, beta_;	
};




struct VS_INPUT_SKY
{
	float4 position : POSITION;
	float4 normal 	: NORMAL;
	float2 texco	: TEXCOORD;
};

struct VS_OUTPUT_SKY
{
	float4 position	: POSITION;
	float3 texco	: TEXCOORD;
};

struct PS_INPUT_SKY
{
	float3 texco	: TEXCOORD0;
};

struct PS_OUTPUT_SKY
{
	float4 color : COLOR0;
};




OMaterial mtrl;
OLight light;

float4 camera_position;

matrix worldMatrix;
matrix viewMatrix;
matrix projMatrix;

matrix invMVP;
matrix skyRotation;

float4 skyLerper;
float4 leafLerper;

// textures
Texture t0, t1;
sampler2D Tex0 = sampler_state {
	Texture = <t0>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler2D Tex1 = sampler_state {
	Texture = <t1>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

// cube textures
Texture sky0, sky1, sky2;
samplerCUBE Sky0 = sampler_state {
	Texture = <sky0>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};
samplerCUBE Sky1 = sampler_state {
	Texture = <sky1>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};
samplerCUBE Sky2 = sampler_state {
	Texture = <sky2>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};
// phong光照模型计算
float4 ComputePhongLight(float4 P, float4 N)
{
	// Emissive
	float4 EmissiveValue = mtrl.emissive;
	
	// Ambient
	float4 AmbientValue = float4(mtrl.ambient.xyz * light.ambient.xyz, 0.0f);
	
	// Diffuse
	float4 L = -light.direction;
	L.w = 0;
	L = normalize(L);
	float4 DiffuseValue = mtrl.diffuse * light.color * max(dot(L, N), 0);
	
	// Specular
	float4 V = camera_position - P;
	V.w = 0;
	V = normalize(V);
	float4 H = normalize(L + V);
	float fSpe = pow( max(dot(H, N), 0), mtrl.shinies);
	if(fSpe < 0)
		fSpe = 0;	
	float4 SpecularValue = mtrl.specular * light.color * fSpe;
	
	// Combine
	float4 CombineValue = EmissiveValue + AmbientValue + DiffuseValue + SpecularValue;
	return CombineValue;		
}

matrix RotAxisAngle(float3 axis, float angle)
{
//	axis = normalize(axis);
	float fc = cos(angle), fs = sin(angle);
	
	float u = axis.x, v = axis.y, w = axis.z;
	
	matrix m = {
		fc+u*u*(1-fc), u*v*(1-fc)+w*fs, u*w*(1-fc)-v*fs, 0,
		u*v*(1-fc)-w*fs, fc+v*v*(1-fc), w*v*(1-fc)+u*fs, 0,
		u*w*(1-fc)+v*fs, v*w*(1-fc)-u*fs, fc+w*w*(1-fc), 0,
		0,				0,				  0,				1
	};
	return m;
}




// 树干，房屋等的shader
VS_OUTPUT MeshVS(VS_INPUT input)
{
	// position
	VS_OUTPUT output = (VS_OUTPUT)0;	
	
	matrix temp = mul(worldMatrix, viewMatrix);
	matrix mvpMatrix = mul(temp, projMatrix);
	
	output.position = mul( input.position, mvpMatrix);	
	// color
	output.color = ComputePhongLight(input.position, input.normal);

	output.texco = input.texco;
	// texco for 3dsmax
	output.texco.y = 1 - output.texco.y;
	return output;
}

PS_OUTPUT MeshPS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
//	output.color = tex2D(Dif, input.texCoord) * 0.8 + input.color * 0.2;
	output.color = tex2D(Tex0, input.texco);
	
	clip(output.color.a - 0.1);
	
	
	return output;
}


// 地形的shader
VS_OUTPUT TerrainVS(VS_INPUT input)
{
	// position
	VS_OUTPUT output = (VS_OUTPUT)0;	
	
	matrix temp = mul(worldMatrix, viewMatrix);
	matrix mvpMatrix = mul(temp, projMatrix);
	
	output.position = mul( input.position, mvpMatrix);	
	// color
	output.color = ComputePhongLight(input.position, input.normal);

	output.texco = input.texco;
	// texco for 3dsmax
	output.texco.y = 1 - output.texco.y;
	return output;
}

PS_OUTPUT TerrainPS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.color = tex2D(Tex0, input.texco);
	return output;
}


// 天空盒的shader
VS_OUTPUT_SKY SkyVS(VS_INPUT_SKY input)
{
	VS_OUTPUT_SKY output = (VS_OUTPUT_SKY)0;
	
	output.position = input.position;
	output.position.w = 1;
	
	float4 temp = mul(input.position, invMVP);
	
	temp = mul(temp, skyRotation);
	
	output.texco = normalize(temp.xyz);
	
	return output;
	
}

PS_OUTPUT_SKY SkyPS(PS_INPUT_SKY input)
{
	PS_OUTPUT_SKY output = (PS_OUTPUT_SKY)0;
	
	float4 c0 = texCUBE(Sky0, input.texco);
	float4 c1 = texCUBE(Sky1, input.texco);
	float4 c2 = texCUBE(Sky2, input.texco);
	
	output.color = skyLerper[0] * c0 + skyLerper[1] * c1 + skyLerper[2] * c2;
	
	
	
//	output.color = texCUBE(Sky, input.texco);
//	output.color = float4(1, 0, 0, 1);

	return output;
}


// 树叶的shader
VS_OUTPUT LeafVS(VS_INPUT_LEAF input)
{
	float3 onor = normalize(input.normal.xyz);
	
	float3 up = float3(0, 0, 1);
	
	float3 axis;
	
	if(length(onor - up) < FLOAT_ZERO)
		axis = float3(1, 0, 0);
	else
		axis = normalize(cross(onor, up));
	
	float alpha = input.para1.x;
	matrix mr = RotAxisAngle(onor, alpha);
	
	
	float beta = input.para3.y;
	matrix mn = RotAxisAngle(axis, beta);


	
	float3 ov = float3(input.para2.x, input.para2.y, input.para3.x);
	
//	ov = mul(ov, mul(mn, mr));
	ov = mul(ov, mul(mr,mn));
//	ov = mul(ov, mn);
	
	ov *= input.para1.y;
	
	float4 pos;
	pos.xyz = input.center.xyz + ov;
	pos.w = 1;
	
	VS_OUTPUT output = (VS_OUTPUT)0;		
	matrix mvpMatrix = mul(mul(worldMatrix, viewMatrix), projMatrix);
	output.position = mul(pos, mvpMatrix);
	
	output.texco.x = input.texco.x;
	output.texco.y = 1 - input.texco.y;
	
	return output;

}

PS_OUTPUT LeafPS(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
//	output.color = tex2D(Dif, input.texCoord) * 0.8 + input.color * 0.2;

	output.color = tex2D(Tex0, input.texco) * leafLerper[0] + tex2D(Tex1, input.texco) * leafLerper[1];	
	clip(output.color.a - 0.5);	

	return output;
}


technique Tech
{
	pass P0
	{
		vertexShader = compile vs_3_0 MeshVS();
		pixelShader = compile ps_3_0 MeshPS();	
		
		CullMode = NONE;
	}
	
	pass P1
	{
		vertexShader = compile vs_3_0 TerrainVS();
		pixelShader = compile ps_3_0 TerrainPS();
		
	}
	
	pass P2
	{
		vertexShader = compile vs_3_0 SkyVS();
		pixelShader = compile ps_3_0 SkyPS();

	}
	
	pass P3
	{
		VertexShader = compile vs_3_0 LeafVS();
		PixelShader = compile ps_3_0 LeafPS();
		
		CullMode = NONE;
	}
}


