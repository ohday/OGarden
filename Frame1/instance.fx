matrix mat_view;
matrix mat_model;
matrix mat_projection;

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
};

struct VS_OUTPUT
{
	float4 position	: POSITION;
	float4 color	: COLOR;
};

struct PS_INPUT
{
	float2 texCoord : TEXCOORD0;	
};

struct PS_OUTPUT
{
	float4 color : COLOR0;
}


OMaterial mtrl;
OLight light;
float4 eye_position;


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
	float4 V = eye_position - P;
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

VS_OUTPUT vsmain(VS_INPUT input)
{
	matrix mat_mvp = mat_model * mat_view * mat_projection;

	// position
	VS_OUTPUT output = (VS_OUTPUT)0;	
	output.position = mul( input.position, mat_mvp);
	
	// color
	output.color = ComputePhongLight(input.position, input.normal);
	return output;
}


technique Tech
{
	pass P0
	{
		vertexShader = compile vs_3_0 vsmain();
//		pixelShader = compile ps_3_0 psmain();
		
	}
}



