
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
	float2 texco 	:	TEXCOORD0;	// uv
	
	float2 para1 	:	TEXCOORD1;	// rotW, rollW
	float2 para2	:	TEXCOORD2;	// yV, delatyTime
	float2 para3	:	TEXCOORD3;	// xPhi, zPhi
	float2 para4	:	TEXCOORD4;	// xW, zW
	float2 para5	:	TEXCOORD5;	// xScalar, zScalar
	float2 para6	:	TEXCOORD6;	// ovx, ovy
	float2 para7	:	TEXCOORD7;	// ovz, temp
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




OMaterial mtrl;					// 材质
OLight light;					// 光照

float leafScalar;				// 树叶放缩因子
float leafFallTime;				// 树叶飘落时间
float4 wind;					// 风

float4 camera_position;			// 摄像机位置

matrix worldMatrix;				// 世界矩阵
matrix viewMatrix;				// 观察矩阵
matrix projMatrix;				// 投影矩阵

matrix invMVP;					// 反mvp矩阵
matrix skyRotation;				// 天空盒旋转矩阵

float4 skyLerper;				// 天空纹理lerper
float4 leafLerper;				// 树叶纹理lerper

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

// 风的计算
float WindMotion(float t)
{
	// 调参后的风函数为：-1.04072 + 1.9 x - 2.7864 Cos[0.24 x] + 1.92712 Cos[0.6 x] + 0.861934 Sin[0.24 x] + 0.534998 Sin[0.6 x]
	
	// p用来调整多大范围在前进方向，[0~1.9],w调整风强
	float p = 1.1, w = 0.3;
	
	
	
	float ret = -1.04072 + p*t - 2.7864*cos(0.24*t) + 1.92712*cos(0.6*t) + 0.861934*sin(0.24*t) + 0.534998*sin(0.6*t);
	
	// 我们调整一下风强
	ret *= 0.4;
	return ret;
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

	float t = leafFallTime - input.para2.y;
	if(t < 0)
		t = 0;


	float xPhi = input.para3.x, zPhi = input.para3.y;
	float xW = input.para4.x, zW = input.para4.y;	
	float xScalar = input.para5.x, zScalar = input.para5.y;


	float4 leafCenter_World = mul(input.center, worldMatrix);
	
	float3 leafNormal_World = normalize(mul(input.normal, worldMatrix).xyz);
	float3 up_World = float3(0, 1, 0);
	float3 axis_World;	
	if(length(leafNormal_World - up_World) < FLOAT_ZERO)
		axis_World = float3(1, 0, 0);
	else
		axis_World = normalize(cross(leafNormal_World, up_World));
	
	// 匀速直线向下
	float realHeight = leafCenter_World.y - (t * input.para2.x);
	
	// 判断是否已经落到地面，这里后面可以加入渐渐消失的特效
	// 同时，这里只是将y高度为0作为停止条件，后期希望可以加上高度场的贴图，判断是否大于贴图中该值
	if(realHeight < 0)
	{
		t = leafCenter_World.y / input.para2.x;
//		t = 0;
		leafCenter_World.y = 0;
	}
	else
	{
		leafCenter_World.y = realHeight;
	}
		
	// xz方向
	leafCenter_World.x = leafCenter_World.x + (xScalar / xW) * (cos(xPhi) - cos(xW * t + xPhi));
	leafCenter_World.z = leafCenter_World.z + (zScalar / zW) * (cos(zPhi) - cos(zW * t + zPhi));

	// 风：
	float windDis = WindMotion(input.para2.y + t) - WindMotion(input.para2.y);
	leafCenter_World.x += (windDis * wind.x);
	leafCenter_World.z += (windDis * wind.z);
	
	// rotation
	float alpha = input.para1.x * t;
	matrix mr = RotAxisAngle(leafNormal_World.xyz, alpha);
	
	// roll
	float beta = input.para1.y * t;
	matrix mn = RotAxisAngle(axis_World, beta);
	
	// compute
	float3 ov = float3(input.para6.x, input.para6.y, input.para7.x);
	ov *= leafScalar;
	
	ov = mul(ov, worldMatrix);

	ov = mul(ov, mul(mr, mn));
	
	float4 pos;
	pos.xyz = leafCenter_World.xyz + ov;
	pos.w = 1;
	
	VS_OUTPUT ret = (VS_OUTPUT)0;		
	matrix vpMatrix = mul(viewMatrix, projMatrix);
	
	ret.position = mul(pos, vpMatrix);
	
	ret.texco.x = input.texco.x;
	ret.texco.y = 1 - input.texco.y;
	
	return ret;
}

// VS_OUTPUT LeafVS(VS_INPUT_LEAF input)
// {
	





// /*	float3 leafCenter;

	// OLeafMotion lm = leafMotionParas;

	
	// leafCenter.z = input.center.z - lm.yv * fallTime;
	
	// leafCenter.x = input.center.x + (lm.xScalar / lm.xW) * (cos(lm.xPhi) - cos(lm.xW * fallTime + lm.xPhi));
	// leafCenter.x = input.center.x + (lm.xScalar / lm.xW) * (cos(lm.xPhi) - cos(lm.xW * fallTime + lm.xPhi));
	

// */


	// float3 onor = normalize(input.normal.xyz);
	
	// float3 up = float3(0, 0, 1);
	
	// float3 axis;
	
	// if(length(onor - up) < FLOAT_ZERO)
		// axis = float3(1, 0, 0);
	// else
		// axis = normalize(cross(onor, up));
	
	// float alpha = input.para1.x;
	// matrix mr = RotAxisAngle(onor, alpha);
	
	
	// float beta = input.para3.y;
	// matrix mn = RotAxisAngle(axis, beta);


	
	// float3 ov = float3(input.para2.x, input.para2.y, input.para3.x);
	
	////ov = mul(ov, mul(mn, mr));
	// ov = mul(ov, mul(mr,mn));
	////ov = mul(ov, mn);
	
	// ov *= input.para1.y;
	
	// float4 pos;
	// pos.xyz = input.center.xyz + ov;
	// pos.w = 1;
	
	// VS_OUTPUT output = (VS_OUTPUT)0;		
	// matrix mvpMatrix = mul(mul(worldMatrix, viewMatrix), projMatrix);
	// output.position = mul(pos, mvpMatrix);
	
	// output.texco.x = input.texco.x;
	// output.texco.y = 1 - input.texco.y;
	
	// return output;

// }

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


