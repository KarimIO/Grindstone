#version 330 core

out vec4 outColor;

in vec3 fragCoord;

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
    vec3 eyepos;
    float time;
    mat4 invProj;
    mat4 invView;
	vec4 resolution;
} ubo;

uniform sampler2D stars;
uniform sampler2D clouds_day;
uniform sampler2D clouds_noise;
uniform sampler2D color_lut;

vec3 suncolor = vec3(1.0f, 0.8f, 0.4f);

vec3 getSunPos() {
    float time = ubo.time / 5.0f;

    vec3 sundir = vec3(0,1,0);
    sundir.x = sin(time);
    sundir.y = cos(time);

    return normalize(sundir);
}

float clouds(in vec2 coord) {
    float t = 4.0f;

    float sunheighty = getSunPos().y;
    float sunheight = clamp(sunheighty * 0.5 + 0.5, 0, 1);

    vec2 t_coord = vec2(t + coord.x, coord.y);
    
    float cloud_d = texture(clouds_day, t_coord).r;
    float cloud_m = texture(clouds_noise, t_coord).r;
    float cloud_f = mix(cloud_d, cloud_m, coord.y);

    float cloud_v = (1 - coord.y) * 4;
    cloud_v = (clamp(cloud_v, 0, 1));

    return sunheight * cloud_v * cloud_f * clamp(fragCoord.y * 400.0f, 0, 1);
}

#define PI 3.1415926535897932384626433832795

void rimColor(out vec3 color, out float alpha, in float sundot, in vec3 coord, in vec3 cloudcolor) {
    float horizon = coord.y;

    vec2 cloud_nc = normalize(coord.xz);
    float cloud_x = atan(cloud_nc.y / cloud_nc.x)/PI+0.5f;
    cloud_x = (coord.x < 0) ? (cloud_x) : (cloud_x+1.0f);
    cloud_x /= 2.0f;
    vec2 cloudcoord = vec2(cloud_x + ubo.time / 1000.0f, 1-asin(abs(coord.y))/PI*2);

    float cloudpow = texture(clouds_noise, cloudcoord).r;
    cloudpow = mix(1.0f, 4.0f, cloudpow);

    float clouds = clouds(cloudcoord);
    // clouds = pow(clouds, cloudpow);
    alpha = clouds * clouds;

    float cloud_sunmask_v = clamp(pow(sundot*1.3f,10), 0, 1);
    vec3 cloud_sunmask = suncolor * clouds;

    vec3 cloud_f = clouds * cloudcolor;
    color = cloud_sunmask + cloud_f;
}

vec3 skycolor(in vec3 coord, in vec3 zenithcolor, in vec3 horizoncolor) {
    // Params
    float sunheight = getSunPos().y;
    float horizonfalloff = mix(3, 7, abs(sunheight));
    sunheight = (sunheight < 0) ? abs(sunheight) : 0;
    float starbrightness = 1.0; // 0.1
    
    // Textures
    vec3 stars = texture(stars, coord.xz*2.5).rgb;

    float horizon = (1 - coord.y);
    horizon = pow(horizon, horizonfalloff);
    horizon = clamp(horizon, 0, 1);

    vec3 stars_final = clamp(coord.y, 0, 1) * stars  * sunheight;
    vec3 zenith_final = zenithcolor + stars_final;
    return mix(zenith_final, horizoncolor, horizon);
}

vec3 sunmaskcolor(in float sundot) {
    float sunbrightness = 50.0f;
    float sunradius = 0.0003;
    
    vec3 suncolor_b = sunbrightness * suncolor;
    float sunmask = (sundot > 1.0f-sunradius) ? 1.0f : 0.0f;
    //sunmask = clamp(sunmask - (1-sunradius), 0, 1) / sunradius;
    //sunmask = pow(sunmask, 0.4);
    
    return sunmask*suncolor_b;
}

vec3 fullskycolor(in float sundot, in vec3 coord, in vec3 zenithcolor, in vec3 horizoncolor) {
    return sunmaskcolor(sundot) + skycolor(coord, zenithcolor, horizoncolor);
}

void main() {
    vec3 coord = normalize(fragCoord);
    vec3 sundir = getSunPos();
    float sunheight = sundir.y * 0.49f + 0.50f;

    vec3 zenithcolor = textureLod(color_lut, vec2(sunheight, 0.0f), 0).rgb;
    vec3 horizoncolor = textureLod(color_lut, vec2(sunheight, 3.0f/8.0f), 0).rgb;
    vec3 cloudcolor = textureLod(color_lut, vec2(sunheight, 7.0f/8.0f), 0).rgb;

    float sundot = clamp(dot(coord, sundir), 0, 1);

    float alpha = 1.0f;
    vec3 rimcolor = vec3(0.0f);
    rimColor(rimcolor, alpha, sundot, coord, cloudcolor);
    vec3 skycolor = fullskycolor(sundot, coord, zenithcolor, horizoncolor);

    alpha = clamp(alpha, 0, 1);
    
    vec3 overallcolor = mix(skycolor, rimcolor * 1.5, alpha);
	outColor = vec4(overallcolor, 1);
}