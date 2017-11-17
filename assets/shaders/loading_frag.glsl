#version 330 core

out vec4 finalColor;

in vec2 fragTexCoord;

uniform sampler2D logo;

layout(std140) uniform UniformBufferObject {
    float aspect;
    float time;
} ubo;

float Dist(vec2 d) {
    return sqrt(d.x*d.x + d.y*d.y);
}

float CalcSphere(vec2 pos, float size) {
    vec2 pos2 = vec2(pos.x * ubo.aspect, pos.y);
    float dist = Dist(pos2);
    return (dist < size) ? 1.0f : 0.0f;
}

void main() {
    vec2 res = vec2(1366, 768);
    float h = res.y * 0.28;
    vec2 picCoord = vec2((res.x - h), (res.y - h))/2;
    vec2 texCoord = fragTexCoord * res;
    texCoord -= picCoord;
    texCoord /= h;

    vec4 color = texture(logo, texCoord);
    if (texCoord.x < 0 || texCoord.x > 1 || texCoord.y < 0 || texCoord.y > 1)
        color = vec4(0,0,0,1);

    vec4 background = vec4(0.211, 0.306, 0.341, 1.0);
    float cs = 0.0f;
    if (ubo.time > 0.0f) {
        const float circle_dist = 0.15;
        const uint num_orbs = 10u;
        float offset = sin(ubo.time * 2 / 3.14159) * 0.5 + 1;
        for (uint i = 0u; i < num_orbs; i++) {
            float f = float(i) / num_orbs;
            float off = ubo.time + i;
            off *= -0.2;
            vec2 circle_pos = vec2(0.5 + cos(off) * circle_dist / ubo.aspect, 0.5 + sin(off) * circle_dist);
            cs += CalcSphere(circle_pos - fragTexCoord, 0.005 * sqrt(f)) * f;
        }
    }
    vec4 c = vec4(cs, cs, cs, 1);
    finalColor =  c + background + color;
}