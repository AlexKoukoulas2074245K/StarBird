#version 300 core

precision mediump float;

in vec2 uv_frag;

uniform sampler2D tex;
uniform float tex_offset;

out vec4 frag_color;

void main()
{
    float finalUvX = uv_frag.x;
    float finalUvY = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(finalUvX, tex_offset + finalUvY));
    
    if (frag_color.a < 0.2) discard;
}

