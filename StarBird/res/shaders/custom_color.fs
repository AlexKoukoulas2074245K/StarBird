#version 300 core

precision mediump float;

in vec2 uv_frag;

uniform sampler2D tex;
uniform vec4 custom_color;
out vec4 frag_color;

void main()
{
    float finalUvX = uv_frag.x;
    float finalUvY = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(finalUvX, finalUvY));
    
    if (frag_color.a < 0.25) discard;
    
    frag_color *= custom_color;
    frag_color = min(vec4(1.0f), frag_color);
}
