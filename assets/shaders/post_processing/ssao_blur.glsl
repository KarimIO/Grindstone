#version 140

uniform sampler2D SSAOin;
uniform sampler2D texAlbedo;

const int uBlurSize = 4;

in vec2 UV;

out vec3 SSAOout;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(SSAOin, 0));
    float result = 0.0;
    for (int i = 0; i < uBlurSize; ++i) {
        for (int j = 0; j < uBlurSize; ++j) {
            vec2 offset = vec2(float(i), float(j)) * texelSize;
            result += texture(SSAOin, UV + offset).r;
        }
    }

    float strength = result / float(uBlurSize * uBlurSize);
    SSAOout = texture(texAlbedo, UV).rgb * strength * strength * vec3(0.052, 0.082, 0.098);
}