#version 400

in vec2 vert_UV;
in float vert_TileLayer;
in vec3 vert_TileFGColor;
in vec3 vert_TileBGColor;

uniform sampler2DArray u_TileTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
	float a = texture(u_TileTexture, vec3(vert_UV, vert_TileLayer)).r;
	vec3 color = mix(vert_TileBGColor, vert_TileFGColor, a);
	out_Color = vec4(color, 1.0);
}