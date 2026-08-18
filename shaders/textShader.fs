#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform float alphaMul;   // 1.0 for glyphs; < 1.0 for translucent solid backgrounds

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, alphaMul) * sampled;
}