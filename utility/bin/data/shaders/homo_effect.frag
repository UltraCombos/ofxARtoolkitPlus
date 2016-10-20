#pragma include <header.glsl>

uniform sampler2D tex_input;
uniform sampler2D tex_mask;
uniform mat4 homo_mat4;

in block {
	vec2 texCoord;
} In;

layout(location = 0) out vec4 fragColor;

vec2 find_warped_point(vec2 pnt)
{
	mat3 mat3Homo = mat3(homo_mat4);
	
	vec3 srcPnt = vec3(pnt.xy, 1.0);
	vec3 dstPnt = mat3Homo * srcPnt;
	dstPnt.x = dstPnt.x / dstPnt.z;
	dstPnt.y = dstPnt.y / dstPnt.z;
	return dstPnt.xy;
}

void main(void)
{
	vec2 tex_coord = find_warped_point(In.texCoord);
	vec4 color = texture(tex_input, tex_coord);
	float mask = texture(tex_mask, In.texCoord).r;
    fragColor = vec4(color.rgb, mask);
}