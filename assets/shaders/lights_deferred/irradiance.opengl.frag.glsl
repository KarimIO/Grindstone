#version 330 core

#define M_PI 3.1415926535897932384626433832795

layout(std140) uniform DefferedUBO {
    mat4 invView;
    mat4 invProj;
    vec4 eyePos;
	vec4 resolution;
	float time;
} ubo;

layout(std140) uniform SphericalHarmonicsBuffer {
	mat4 pvm;
    vec3 sh0[9];
    vec3 sh1[9];
} spherical_harmonic_list;

out vec4 outColor;

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;

vec4 ViewPosFromDepth(float depth, vec2 TexCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ubo.invProj * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromViewPos(vec4 view) {
    vec4 worldSpacePosition = ubo.invView * view;

    return worldSpacePosition.xyz;
}

vec3 WorldPosFromDepth(float depth, vec2 TexCoord) {
    return WorldPosFromViewPos(ViewPosFromDepth(depth, TexCoord));
}

in vec3 viewRay;

void mixSH(float fac, vec3 sh1[9], vec3 sh2[9], out vec3 sh_out[9]) {
    fac = clamp(fac, 0, 1);
    sh_out[0] = mix(sh1[0], sh2[0], fac);
	sh_out[1] = mix(sh1[1], sh2[1], fac);
	sh_out[2] = mix(sh1[2], sh2[2], fac);
	sh_out[3] = mix(sh1[3], sh2[3], fac);
	sh_out[4] = mix(sh1[4], sh2[4], fac);
	sh_out[5] = mix(sh1[5], sh2[5], fac);
	sh_out[6] = mix(sh1[6], sh2[6], fac);
	sh_out[7] = mix(sh1[7], sh2[7], fac);
	sh_out[8] = mix(sh1[8], sh2[8], fac);
}

// https://patapom.com/blog/SHPortal/
vec3 evaluateSHIrradiance( vec3 _Direction, float _CosThetaAO,  vec3 _SH[9] ) {
    float   t2 = _CosThetaAO*_CosThetaAO;
    float   t3 = t2*_CosThetaAO;
    float   t4 = t3*_CosThetaAO;
    float   ct2 = 1.0 - t2; 

    float       c0 = 0.88622692545275801364908374167057 * ct2;          // 1/2 * sqrt(PI) * (1-t^2)
    float       c1 = 1.02332670794648848847955162488930 * (1.0-t3);     // sqrt(PI/3) * (1-t^3)
    float       c2 = 0.24770795610037568833406429782001 * (3.0 * (1.0-t4) - 2.0 * ct2); // 1/16 * sqrt(5*PI) * [3(1-t^4) - 2(1-t^2)]
    const float sqrt3 = 1.7320508075688772935274463415059;

    float   x = _Direction.x;
    float   y = _Direction.y;
    float   z = _Direction.z;

    return  max( vec3(0.0), c0 * _SH[0]                                       // c0.L00
            + c1 * (_SH[1]*y + _SH[2]*z + _SH[3]*x)                     // c1.(L1-1.y + L10.z + L11.x)
            + c2 * (_SH[6]*(3.0*z*z - 1.0)                              // c2.L20.(3z²-1)
                + sqrt3 * (_SH[8]*(x*x - y*y)                           // sqrt(3).c2.L22.(x²-y²)
                    + 2.0 * (_SH[4]*x*y + _SH[5]*y*z + _SH[7]*z*x)))    // 2sqrt(3).c2.(L2-2.xy + L2-1.yz + L21.zx)
        );
 }

void main() {
    vec2 fragTexCoord = gl_FragCoord.xy / ubo.resolution.xy;
	float depth = texture(gbuffer3, fragTexCoord).r;
	/*float near = 0.1;
    float far = 100;
    float ProjectionA = far / (far - near);
    float ProjectionB = (-far * near) / (far - near);
    depth = ProjectionB / ((depth - ProjectionA));*/
    vec3 position = viewRay * depth;

	vec3 Position = WorldPosFromDepth(depth, fragTexCoord);
	vec3 Normal = texture(gbuffer1, fragTexCoord).rgb;
	vec3 Albedo = texture(gbuffer0, fragTexCoord).rgb;

    vec3 sh[9] = spherical_harmonic_list.sh0;
    mixSH(fragTexCoord.y, spherical_harmonic_list.sh0, spherical_harmonic_list.sh1, sh);

    float AO = 1;
    float cThetaAO = cos( M_PI/2 * AO );
    outColor = vec4(Albedo * evaluateSHIrradiance(Normal, cThetaAO, sh), 1);
}