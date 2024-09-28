#version 400

uniform vec4 u_Color;
uniform sampler2D u_Texture;

in vec2 vert_UV;

layout(location = 0) out vec4 out_Color;

void main()
{
	out_Color = texture(u_Texture, vert_UV);
}
