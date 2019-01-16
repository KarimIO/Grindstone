#version 330 core

uniform sampler2D img;
uniform sampler2D lut;

in vec2 fragTexCoord;
out vec4 outcolor;


#define MAXCOLOR 15.0 
#define COLORS 16.0
#define WIDTH 256.0 
#define HEIGHT 16.0

vec4 translateCoords(sampler2D texture, vec3 uv, float width) {
    float sliceSize = 1.0 / width;              // space of 1 slice
    float slicePixelSize = sliceSize / width;           // space of 1 pixel
    float sliceInnerSize = slicePixelSize * (width - 1.0);  // space of width pixels
    float zSlice0 = min(floor(uv.z * width), width - 1.0);
    float zSlice1 = min(zSlice0 + 1.0, width - 1.0);
    float xOffset = slicePixelSize * 0.5 + uv.x * sliceInnerSize;
    float s0 = xOffset + (zSlice0 * sliceSize);
    float s1 = xOffset + (zSlice1 * sliceSize);
    vec4 slice0Color = texture2D(texture, vec2(s0, uv.y));
    vec4 slice1Color = texture2D(texture, vec2(s1, uv.y));
    float zOffset = mod(uv.z * width, 1.0);
    vec4 result = mix(slice0Color, slice1Color, zOffset);
    return result;
}

void main() {
    vec4 original = texture(img, fragTexCoord);
    vec4 final;
    final.rgb = original.rgb;
    final = translateCoords(lut, final.rgb, 16);
    final.a = original.a;
    outcolor = final;
}