#version 400

in vec2 vert_UV;

uniform sampler2D u_Texture;

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = texture(u_Texture, vert_UV);
}
