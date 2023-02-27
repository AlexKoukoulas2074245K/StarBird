#version 300 core

precision mediump float;

in vec2 uv_frag;
in vec3 interp_pos;

uniform sampler2D tex;
uniform vec4 custom_color;
out vec4 frag_color;

void main()
{
    float finalUvX = uv_frag.x;
    float finalUvY = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(finalUvX, finalUvY));
    
    if (frag_color.a < 0.2) discard;
    if (interp_pos.y < 0.1) discard;
    if (interp_pos.y > 0.75) discard;
    
    frag_color = custom_color;
    frag_color.w = clamp(0.0f, 1.0f, interp_pos.y + 0.4);
}
