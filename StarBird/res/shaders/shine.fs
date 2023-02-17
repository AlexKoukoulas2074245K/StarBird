#version 300 core

precision mediump float;

in vec2 uv_frag;

uniform sampler2D tex;
uniform sampler2D shineTex;

uniform float shine_x_offset;

out vec4 frag_color;

void main()
{
    float finalUvX = uv_frag.x;
    float finalUvY = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(finalUvX, finalUvY));
    
    if (frag_color.a < 0.2) discard;
    
    vec4 shine_color = texture(shineTex, vec2(finalUvX + shine_x_offset, finalUvY));
    frag_color.rgba += shine_color.a;
    
    frag_color.r = min(1.0f, frag_color.r);
    frag_color.g = min(1.0f, frag_color.g);
    frag_color.b = min(1.0f, frag_color.b);
    frag_color.a = min(1.0f, frag_color.a);
}
