#version 300 core

precision mediump float;

in vec2 uv_frag;

uniform sampler2D tex;
uniform float custom_alpha;
out vec4 frag_color;

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0f - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));
    
    if (frag_color.a < 0.25f) discard;
    else frag_color.w = max(0.0f, custom_alpha);
}
