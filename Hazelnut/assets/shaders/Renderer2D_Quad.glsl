//Basic 2D quad shader
#type vertex
#version 450 core
			
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TextureCoord;
layout(location = 3) in float a_TextureIndex;
layout(location = 4) in float a_TileFactor;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TextureCoord;
	float TileFactor;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat float v_TextureIndex;
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.Color = a_Color;
	Output.TextureCoord = a_TextureCoord;
	Output.TileFactor = a_TileFactor;
	v_TextureIndex = a_TextureIndex;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
			
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec4 Color;
	vec2 TextureCoord;
	float TileFactor;
};

layout (location = 0) in VertexOutput Input;
layout (location = 3) in flat float v_TextureIndex;
layout (location = 4) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
	vec4 texColor = Input.Color;

	switch(int(v_TextureIndex))
	{
		case  0: texColor *= texture(u_Textures[ 0], Input.TextureCoord * Input.TileFactor); break;
		case  1: texColor *= texture(u_Textures[ 1], Input.TextureCoord * Input.TileFactor); break;
		case  2: texColor *= texture(u_Textures[ 2], Input.TextureCoord * Input.TileFactor); break;
		case  3: texColor *= texture(u_Textures[ 3], Input.TextureCoord * Input.TileFactor); break;
		case  4: texColor *= texture(u_Textures[ 4], Input.TextureCoord * Input.TileFactor); break;
		case  5: texColor *= texture(u_Textures[ 5], Input.TextureCoord * Input.TileFactor); break;
		case  6: texColor *= texture(u_Textures[ 6], Input.TextureCoord * Input.TileFactor); break;
		case  7: texColor *= texture(u_Textures[ 7], Input.TextureCoord * Input.TileFactor); break;
		case  8: texColor *= texture(u_Textures[ 8], Input.TextureCoord * Input.TileFactor); break;
		case  9: texColor *= texture(u_Textures[ 9], Input.TextureCoord * Input.TileFactor); break;
		case 10: texColor *= texture(u_Textures[10], Input.TextureCoord * Input.TileFactor); break;
		case 11: texColor *= texture(u_Textures[11], Input.TextureCoord * Input.TileFactor); break;
		case 12: texColor *= texture(u_Textures[12], Input.TextureCoord * Input.TileFactor); break;
		case 13: texColor *= texture(u_Textures[13], Input.TextureCoord * Input.TileFactor); break;
		case 14: texColor *= texture(u_Textures[14], Input.TextureCoord * Input.TileFactor); break;
		case 15: texColor *= texture(u_Textures[15], Input.TextureCoord * Input.TileFactor); break;
		case 16: texColor *= texture(u_Textures[16], Input.TextureCoord * Input.TileFactor); break;
		case 17: texColor *= texture(u_Textures[17], Input.TextureCoord * Input.TileFactor); break;
		case 18: texColor *= texture(u_Textures[18], Input.TextureCoord * Input.TileFactor); break;
		case 19: texColor *= texture(u_Textures[19], Input.TextureCoord * Input.TileFactor); break;
		case 20: texColor *= texture(u_Textures[20], Input.TextureCoord * Input.TileFactor); break;
		case 21: texColor *= texture(u_Textures[21], Input.TextureCoord * Input.TileFactor); break;
		case 22: texColor *= texture(u_Textures[22], Input.TextureCoord * Input.TileFactor); break;
		case 23: texColor *= texture(u_Textures[23], Input.TextureCoord * Input.TileFactor); break;
		case 24: texColor *= texture(u_Textures[24], Input.TextureCoord * Input.TileFactor); break;
		case 25: texColor *= texture(u_Textures[25], Input.TextureCoord * Input.TileFactor); break;
		case 26: texColor *= texture(u_Textures[26], Input.TextureCoord * Input.TileFactor); break;
		case 27: texColor *= texture(u_Textures[27], Input.TextureCoord * Input.TileFactor); break;
		case 28: texColor *= texture(u_Textures[28], Input.TextureCoord * Input.TileFactor); break;
		case 29: texColor *= texture(u_Textures[29], Input.TextureCoord * Input.TileFactor); break;
		case 30: texColor *= texture(u_Textures[30], Input.TextureCoord * Input.TileFactor); break;
		case 31: texColor *= texture(u_Textures[31], Input.TextureCoord * Input.TileFactor); break;
	}

	if (texColor.a == 0.0)
		discard;

	o_Color = texColor;

	o_EntityID = v_EntityID; 
}