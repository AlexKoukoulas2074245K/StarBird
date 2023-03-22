#version 300 core

precision mediump float;

in vec2 uv_frag;
in vec3 frag_unprojected_pos;

uniform sampler2D tex;
uniform vec4 ambient_light_color;
uniform vec4 point_light_colors[32];
uniform vec3 point_light_positions[32];
uniform float point_light_powers[32];
uniform bool affected_by_light;
uniform int active_light_count;
uniform float hue_shift;

out vec4 frag_color;

const vec4  kRGBToYPrime = vec4 (0.299, 0.587, 0.114, 0.0);
const vec4  kRGBToI     = vec4 (0.596, -0.275, -0.321, 0.0);
const vec4  kRGBToQ     = vec4 (0.212, -0.523, 0.311, 0.0);

const vec4  kYIQToR   = vec4 (1.0, 0.956, 0.621, 0.0);
const vec4  kYIQToG   = vec4 (1.0, -0.272, -0.647, 0.0);
const vec4  kYIQToB   = vec4 (1.0, -1.107, 1.704, 0.0);

void main()
{
    float final_uv_x = uv_frag.x;
    float final_uv_y = 1.0 - uv_frag.y;
    
    // Sample and discard sufficiently transparent fragments
    vec4 sample_color = texture(tex, vec2(final_uv_x, final_uv_y));
    if (sample_color.a < 0.2) discard;
    
    // Convert to YIQ
    float YPrime = dot(sample_color, kRGBToYPrime);
    float I      = dot(sample_color, kRGBToI);
    float Q      = dot(sample_color, kRGBToQ);
    
    // Calculate the hue and chroma
    float hue     = atan (Q, I);
    float chroma  = sqrt (I * I + Q * Q);
    
    // Adjust hue
    hue += hue_shift;
    
    // Convert back to YIQ
    Q = chroma * sin (hue);
    I = chroma * cos (hue);

    // Convert back to rgb
    vec4 yIQ   = vec4 (YPrime, I, Q, 0.0);
    sample_color.r = dot(yIQ, kYIQToR);
    sample_color.g = dot(yIQ, kYIQToG);
    sample_color.b = dot(yIQ, kYIQToB);
    
    frag_color = sample_color;
    
    if (affected_by_light)
    {
        vec4 light_accumulator = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        
        for (int i = 0; i < active_light_count; ++i)
        {
            float dst = distance(point_light_positions[i], frag_unprojected_pos);
            float attenuation = point_light_powers[i] / (dst * dst);
                    
            light_accumulator.rgb += (point_light_colors[i] * attenuation).rgb;
        }
        
        frag_color = frag_color * ambient_light_color + light_accumulator;
    }
}
