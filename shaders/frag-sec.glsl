#line 2

precision mediump float;

#define PI 3.14159265
#define TAU 6.28318530

#define NEW_RAYMARCH 1
#define MAX_STEPS 99

#define layer1 vec3(0.55686, 0.98824, 0.99216)
#define layer2 vec3(0.98039, 0.98824, 0.99216)
#define layer3 vec3(0.26275, 0.98824, 0.99216)


out vec4 fragColor;

layout(std140) uniform CamBlock {
    mat4 projection;
    mat4 view;
    vec3 position;
    float far;
    float near;
} cam_block;

uniform vec2 u_resolution;
uniform float u_time;

float snoise(vec3 uv, float res)	// by trisomie21
{
	const vec3 s = vec3(1e0, 1e2, 1e4);

	uv *= res;

	vec3 uv0 = floor(mod(uv, res))*s;
	vec3 uv1 = floor(mod(uv+vec3(1.), res))*s;

	vec3 f = fract(uv); f = f*f*(3.0-2.0*f);

	vec4 v = vec4(uv0.x+uv0.y+uv0.z, uv1.x+uv0.y+uv0.z,
		      	  uv0.x+uv1.y+uv0.z, uv1.x+uv1.y+uv0.z);

	vec4 r = fract(sin(v*1e-3)*1e5);
	float r0 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);

	r = fract(sin((v + uv1.z - uv0.z)*1e-3)*1e5);
	float r1 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);

	return mix(r0, r1, f.z)*2.-1.;
}

// At the end of the day, it's just a quadratic formula
vec2 quadratic(float a, float b, float c, inout bool hit) {
    float d = b * b - 4.0 * a * c;

    if (d < 0.0) {
        hit = false;
        return vec2(-1.0, -1.0); // consistent failure signal
    }

    float sqrt_d = sqrt(d);
    float inv_2a = 0.5 / a;

    float t0 = (-b - sqrt_d) * inv_2a;
    float t1 = (-b + sqrt_d) * inv_2a;

    if (t0 > t1) {
        float tmp = t0;
        t0 = t1;
        t1 = tmp;
    }

    if (t1 < 0.0) {
        hit = false;
        return vec2(-1.0, -1.0);
    }

    hit = true;
    return vec2(t0, t1);
}

// Most basic raytracing example on how raytracing actually works
// TO WRITE: How this works and the basics of raytracing
vec2 sphere(float r, vec3 rayOrigin, vec3 rayDirection, inout bool hit) {
    vec3 oc = rayOrigin;
    const float a = 1.0;

    float b = 2.0 * dot(oc, rayDirection);
    float c = dot(oc, oc) - r * r;
    vec2 disc = quadratic(a, b, c, hit);

    // If the ray hits the sphere, return the distance from both intersections
    return disc;
}

float SDF_sphere(vec3 p, float r) {
    float fade = sqrt(length(2.0 * p));
    float fVal1	= 1.0 - fade;
	float fVal2	= 1.0 - fade;

    float angle = atan(p.y, p.x) / TAU;
    vec3 coord = vec3(angle, fade, u_time * 0.25);

    float new_time = abs(snoise(coord + vec3( 0.0, -u_time * 0.35, u_time * 0.015), 15.0));

    return length(p) - r * (1.0 - 0.5 * sin(new_time * new_time) * cos(new_time * new_time) * fVal1 * fVal2);
}

float SDF_distance(float t, inout int iter, vec3 ray_origin, vec3 ray_direction) {
    while(iter <= MAX_STEPS) {
        vec3 p = ray_origin + t * ray_direction;
        float dist = SDF_sphere(p, 1.5);

        if(dist < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            break;
        }

        t += dist;
        iter++;
    }

    return -1.0;
}

float raymarch(vec3 ray_origin, vec3 ray_direction, float t_i) {
    float t = t_i;

    #if NEW_RAYMARCH == 1
    #define MIN_GROWTH 0.215

    float t_j = cam_block.far;
    float min_dist = cam_block.far;
    int i = int((MAX_STEPS & 1) == 0);

    #if (MAX_STEPS & 1) == 0
    t = SDF_sphere(ray_origin + t * ray_direction, 1.5);

    if(t < cam_block.near) {
        return t;
    }
    #endif

    while(i <= (MAX_STEPS >> 1)) {
        vec3 p_i = ray_origin + t * ray_direction;
        float dist_i = SDF_sphere(p_i, 1.5);

        if(dist_i < cam_block.near) {
            return t;
        }

        if(t > cam_block.far) {
            return -1.0;
        }

        if(dist_i - min_dist > MIN_GROWTH) {
            vec3 p_j = ray_origin + t_j * ray_direction;
            float dist_j = SDF_sphere(p_j, 1.5);

            if((dist_i + dist_j) >= abs(t_j - t)) {
                return -1.0;
            }

            t_j -= dist_j;
        }else {
            t += dist_i;
            dist_i = SDF_sphere(ray_origin + t * ray_direction, 1.5);

            if(dist_i < cam_block.near) {
                return t;
            }

            if(t > cam_block.far) {
                return -1.0;
            }
        }

        min_dist = min(min_dist, dist_i);
        t += dist_i;
        i++;
    }

    return -1.0;

    #else

    int iter = 0;
    t = SDF_distance(t, iter, ray_origin, ray_direction);

    #endif

    return t;
}

vec4 render(vec2 uv) {
    // World View Projection
    vec4 clip = vec4(uv, -1.0, 1.0);
    vec4 eye = inverse(cam_block.projection) * clip;
    eye /= eye.w;

    // Ray Calculation
    vec3 ray_origin = cam_block.position;
    vec3 ray_direction = normalize((inverse(cam_block.view) * vec4(eye.xyz, 0.0)).xyz);

    {
        bool hit = false;
        vec2 t = sphere(1.0, ray_origin, ray_direction, hit);

        if(hit) {
            float glow = exp(-sqrt(t.y - t.x));
            return vec4(vec3(mix(layer1, layer2, 1.0 - glow)), 1.0);
        }
    }

    {
        bool hit = false;
        vec2 t = sphere(2.0, ray_origin, ray_direction, hit);

        if(hit) {
            vec3 p = ray_origin + t.x * ray_direction;
            float m = raymarch(ray_origin, ray_direction, t.x);
            float glow = exp(-sqrt(t.y - t.x) * 0.5);
            float glow_edge = exp(-sqrt(t.y - t.x) * 0.25);

            if(m != -1.0) {
                vec3 p = ray_origin + m * ray_direction;
                vec3 n = normalize(p);

                vec3 glow = mix(layer2, layer3, 1.0 - glow);
                glow = mix(vec3(0.0), glow, glow_edge);
                return vec4(glow, 1.0);
            }
        }
    }

    return vec4(0.0, 0.0, 0.0, 1.0);
}

void main() {
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;

    fragColor = render(uv);
}
