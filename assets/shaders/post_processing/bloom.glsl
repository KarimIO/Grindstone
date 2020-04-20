#version 330 core

in vec2 fragTexCoord;
out vec4 outcolor;

uniform sampler2D lighting;

const bool horizontal= false;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    /*vec4 original = texture(lighting, fragTexCoord);
    original.rgb -= vec3(1);
    outcolor = original;*/

    vec3 def = vec3(0);

    vec2 tex_offset = 1.0 / textureSize(lighting, 0); // gets size of single texel
    vec3 result = (texture(lighting, fragTexCoord).rgb - def) * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += (texture(lighting, fragTexCoord + vec2(tex_offset.x * i, 0.0)).rgb - def) * weight[i];
            result += (texture(lighting, fragTexCoord - vec2(tex_offset.x * i, 0.0)).rgb - def) * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += (texture(lighting, fragTexCoord + vec2(0.0, tex_offset.y * i)).rgb - def) * weight[i];
            result += (texture(lighting, fragTexCoord - vec2(0.0, tex_offset.y * i)).rgb - def) * weight[i];
        }
    }

    result = clamp(result, vec3(0), vec3(1));
    
    outcolor = vec4(result, 1.0);
}