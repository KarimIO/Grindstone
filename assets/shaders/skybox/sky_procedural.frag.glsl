#version 330 core

out vec4 outColor;

in vec3 fragCoord;

uniform sampler2D stars;
uniform sampler2D clouds_day;
uniform sampler2D clouds_noise;

const vec4 suncolor = vec4(1.0f, 0.8f, 0.4f, 1.0f);
const vec4 cloudcolor = vec4(0.717, 0.782, 0.885, 0);

float clouds(in vec2 coord) {
    float t = 4.0f;
    float cloud_opacity = 1.0f;

    vec2 t_coord = vec2(t, 0) + coord;
    
    float cloud_d = texture(clouds_day, t_coord).r;
    float cloud_m = texture(clouds_noise, t_coord).r;
    float cloud_f = mix(cloud_d, cloud_m, coord.y);

    float cloud_v = (1 - coord.y) * 4;
    cloud_v = cloud_opacity * (clamp(cloud_v, 0, 1));

    return cloud_f * cloud_v; // cloud_f * 
}

#define PI 3.1415926535897932384626433832795

void rimColor(out vec4 color, out float alpha, in float sundot, in vec3 coord) {
    float horizon = coord.y;

    vec2 cloud_nc = normalize(coord.xz);
    float cloud_x = atan(cloud_nc.y / cloud_nc.x)/PI+0.5f;
    cloud_x = (coord.x < 0) ? (cloud_x) : (cloud_x+1.0f);
    cloud_x /= 2.0f;
    vec2 cloudcoord = vec2(cloud_x, 1-asin(abs(coord.y))/PI*2);

    float cloudpow = texture(clouds_noise, cloudcoord).r;
    cloudpow = mix(1.0f, 4.0f, cloudpow);

    float clouds = clouds(cloudcoord);
    clouds = pow(clouds, cloudpow);
    alpha = clouds * clouds;

    float cloud_sunmask_v = clamp(pow(sundot*1.3f,10), 0, 1);
    vec4 cloud_sunmask = suncolor * alpha * 0.4;

    vec4 cloud_f = clouds * cloudcolor;
    color = cloud_sunmask + cloud_f;
}

vec4 skycolor(in vec3 coord) {
    // Params
    float horizonfalloff = 3.0f;
    vec4 horizoncolor = vec4(0.94, 1.0, 1.0, 1.0);
    vec4 zenithcolor = vec4(0.0852, 0.154, 0.34, 1.0);
    float sunheight = 1;
    float starbrightness = 0.1;
    
    // Textures
    vec4 stars = texture(stars, fragCoord.xz*2.5);

    float horizon = 1 - (coord.y);
    horizon = pow(horizon, horizonfalloff);
    horizon = clamp(horizon, 0, 1);

    vec4 stars_final = (clamp(coord.y*coord.y, 0, 1)) * sunheight * starbrightness * stars;
    vec4 zenith_final = zenithcolor + stars_final;
    return mix(zenith_final, horizoncolor, horizon);
}

vec4 sunmaskcolor(in float sundot) {
    float sunbrightness = 50.0f;
    float sunradius = 0.0003;
    
    vec4 suncolor_b = sunbrightness * suncolor;
    float sunmask = (sundot > 1.0f-sunradius) ? 1.0f : 0.0f;
    //sunmask = clamp(sunmask - (1-sunradius), 0, 1) / sunradius;
    //sunmask = pow(sunmask, 0.4);
    
    return vec4(sunmask*suncolor_b);
}

vec4 fullskycolor(in float sundot, in vec3 coord) {
    return sunmaskcolor(sundot) + skycolor(coord);
}

void main() {
    vec3 coord = normalize(fragCoord);
    vec3 sundir = vec3(0,1,0);
    sundir = normalize(sundir);
    float sundot = dot(coord, sundir);

    float alpha = 1.0f;
    vec4 rimcolor = vec4(0.0f);
    rimColor(rimcolor, alpha, sundot, coord);
    vec4 skycolor = fullskycolor(sundot, coord);

    alpha = clamp(alpha, 0, 1);
    
    vec4 overallcolor = vec4(1);
	outColor = mix(skycolor, rimcolor, alpha) * overallcolor * 1.5;
}