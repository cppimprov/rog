#version 400

in vec2 in_VertexPosition;
in float in_VertexLerp;
in vec2 in_BorderPosition;
in float in_BorderWidth;
in vec3 in_BorderColor;

uniform vec2 u_TileSize;
uniform mat4 u_MVP;

out vec3 vert_BorderColor;

void main()
{
	vec2 corner = in_VertexPosition * u_TileSize;
	vec2 center = vec2(0.5, 0.5) * u_TileSize;
	vec2 pos = in_BorderPosition + mix(corner, center, in_VertexLerp * (in_BorderWidth / u_TileSize) * 2.0);
	vert_BorderColor = in_BorderColor;
	gl_Position = u_MVP * vec4(pos.x, 0.0, -pos.y, 1.0);
}
