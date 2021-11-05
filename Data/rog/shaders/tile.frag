#version 400

in vec2 vert_UV;
in float vert_TileLayer;

uniform sampler2DArray u_TileTexture;

layout(location = 0) out vec4 out_Color;

void main()
{
	float a = texture(u_TileTexture, vec3(vert_UV, vert_TileLayer)).r;
	out_Color = vec4(vec3(1.0) * a, 1.0);
}