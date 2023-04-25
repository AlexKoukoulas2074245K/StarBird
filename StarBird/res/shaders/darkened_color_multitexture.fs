#version 300 core

precision mediump float;

in vec2 uv_frag;

uniform sampler2D tex;
uniform sampler2D effectTex;

uniform float darken_value;

out vec4 frag_color;

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));
    if (frag_color.a < 0.25) discard;
    
    vec4 effect_color = texture(effectTex, vec2(final_uv_x, final_uv_y));
    if (effect_color.a > 0.1)
    {
        frag_color = effect_color;
    }
    else
    {
        frag_color.rgb *= 0.75f;
    }
    
    frag_color.rgb *= darken_value;
}
