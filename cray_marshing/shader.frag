uniform float time;
uniform vec3 origin;
uniform vec3 ort1;
uniform vec3 ort2;
uniform vec3 left_upper;
uniform vec3 direction;
uniform float w;
uniform float h;
uniform float res_x;
uniform float res_y;

uniform int spheres_count;
uniform vec3 sph_o[100];
uniform float sph_r[100];

uniform int lights_count;
uniform vec3 light_o[100];
uniform vec3 light_color[100];
uniform float light_intensity[100];
uniform float light_stable_distance[100];

vec3 colours[100];

#define PI 3.14159265359
const float inf = 100000;
const float MIN_DIST = 0.0001;
const float DELTA = 0.001;
const int MAX_ITTERS = 128;
const float STEP_COEF = 1;

float MODULE = 0;
vec3 INF_VEC;

const vec3 AMBIENT = vec3(0, 0, 0);

// strange vector operations by kctf

vec2 max_vec(const vec2 first, const vec2 second) {
	if (first.x > second.x) {
		return first;
	} else {
		return second;
	}
}

vec2 min_vec(const vec2 first, const vec2 second) {
	if (first.x < second.x) {
		return first;
	} else {
		return second;
	}
}

// normal operations

float intersection_sdf(float dist_first, float dist_second) {
	return max(dist_first, dist_second);
}

float union_sdf(float dist_first, float dist_second) {
	return min(dist_first, dist_second);
}

float substraction_sdf(float dist_first, float dist_second) {
	return max(dist_first, -dist_second);
}

// smooth operations by hg_sdf

float union_sdf_smooth_hg(float a, float b, float r) {
	vec2 u = max(vec2(r - a.x, r - b.x), vec2(0));
	return max(r, min(a, b)) - length(u);
}

float intersection_sdf_smooth_hg(float a, float b, float r) {
	vec2 u = max(vec2(r + a, r + b), vec2(0));
	return min(-r, max(a, b)) + length(u);
}

float substraction_sdf_smooth_hg(float a, float b, float r) {
	return intersection_sdf_smooth_hg(a, -b, r);
}

// smooth operations by iq

float union_sdf_smooth(float a, float b, float k) {
  float h = clamp(0.5 + 0.5*(b.x-a.x)/k, 0.0, 1.0);
  return mix(b, a, h) - k*h*(1.0-h);
}

float intersection_sdf_smooth(float a, float b, float k) {
  float h = clamp(0.5 - 0.5*(b.x-a.x)/k, 0.0, 1.0);
  return mix(b.x, a.x, h) + k*h*(1.0-h);
}

float substraction_sdf_smooth(float a, float b, float k) {
  float h = clamp(0.5 - 0.5*(a.x+b.x)/k, 0.0, 1.0);
  return mix(a.x, -b.x, h) + k*h*(1.0-h);
}

// point operations by kctf

vec3 moduled_point(vec3 p, float module) {
	if (module == 0.0) {
		return p;
	}
	p = mod(p + module / 2.0, module) - module / 2.0;
	return p;
}

vec3 moduled_point_plane_xy(vec3 p, float module) {
	if (module == 0.0) {
		return p;
	}
	p.xy = mod(p.xy + module / 2.0, module) - module / 2.0;
	return p;
}

vec3 twisted_point(vec3 p, vec3 k) { // remade id code
	vec3 q = p;
	float c = 0.0;
	float s = 0.0;
	if (k.x != 0.0) {
		c = cos(k.x * p.x);
		s = sin(k.x * p.x);
		mat2  m = mat2(c, -s, s, c);
		q = vec3(m * q.yz, q.x);
	}
	if (k.y != 0.0) {
		c = cos(k.y * p.y);
		s = sin(k.y * p.y);
		mat2  m = mat2(c, -s, s, c);
		q = vec3(m * q.xz, q.y);
	}
	if (k.z != 0.0) {
		c = cos(k.z * p.z);
		s = sin(k.z * p.z);
		mat2  m = mat2(c, -s, s, c);
		q = vec3(m * q.xy, q.z);
	}
    return q;
}

vec3 bended_point(vec3 p, vec3 k) { // remade iq code
	vec3 q = p;
	float c = 0.0;
	float s = 0.0;
	if (k.x != 0.0) {
		c = cos(-k.x * p.y);
		s = sin(-k.x * p.y);
		mat2  m = mat2(c, -s, s, c);
		vec2 a = m*p.xy;
		q = vec3(a.y, a.x, p.z);
	}
	if (k.y != 0.0) {
		c = cos(k.y * p.x);
		s = sin(k.y * p.x);
		mat2  m = mat2(c, -s, s, c);
		q = vec3(m*p.xy,p.z);
	}
	if (k.z != 0.0) {
		c = cos(-k.z * p.y);
		s = sin(-k.z * p.y);
		mat2  m = mat2(c, -s, s, c);
		vec2 a = m*p.zy;
		q = vec3(a.y, p.x, a.x);
	}
    return q;
}

vec3 repeated_point(vec3 p, float module, vec3 l) { // by iq
    vec3 q = p - module * clamp(round(p / module), -l, l);
    return q;
}

mat2 rot(float a) {
	float s = sin(a);
	float c = cos(a);
	return mat2(c, -s, s, c);
}

vec3 rotated_point(vec3 p, vec3 ang) {
	p.zy = p.zy * rot(ang.x);
	p.xz = p.xz * rot(ang.y);
	p.xy = p.xy * rot(ang.z);
	return p;
}

// basic shapes distances

float dist_sphere(const vec3 p, float r) {
	return length(p) - r;
}

float dist_box(const vec3 p, vec3 box) {
	vec3 q = abs(p) - box;
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float dist_plane(vec3 p, vec3 n, float h)
{
  return dot(p, normalize(n)) + h;
}

float dist_segment(vec3 p, vec3 a, vec3 b) {
	vec3 ab = b - a;
	float k = clamp(dot(p - a, b - a) / (ab.x * ab.x + ab.y * ab.y + ab.z * ab.z), 0, 1);
	return length(p - a - k * (b - a));
}

float dist_cross(vec3 p, vec3 pos, float size) {
	float box1 = dist_box(p - pos, vec3(size, size, inf * 2.0));
	float box2 = dist_box(p - pos, vec3(size, inf * 2.0, size));
	float box3 = dist_box(p - pos, vec3(inf * 2.0, size, size));
	return union_sdf(union_sdf(box1, box2), box3);
}

float church(vec3 p) {
	float r = 0.075;
	
	float column = dist_box(p - vec3(0, 0, 2), vec3(1.5, 1.5, 4));
	vec3 q = rotated_point(p - vec3(-1.5, 0, 4), vec3(PI / 4, 0, 0));
	float romb = dist_box(q, vec3(1, 2, 2) * 0.4) - r;
	float stolp = dist_box(p - vec3(-1.5, 0, 1), vec3(0.4, 0.75, 3)) - r;
	float base = dist_box(p - vec3(-1.5, 0, -0.5), vec3(0.4, 1.5, 1.5)) - r;

	float clock = dist_sphere(p - vec3(-1.5, 0, 4), 0.65);
	float punkt = dist_box(repeated_point(p - vec3(-1.85, 0, 0.8), 1.5, vec3(0, 1, 2)), vec3(10, 1, 2) * 0.25);

	stolp = union_sdf(stolp, romb);
	stolp = substraction_sdf_smooth(stolp, clock, r);
	stolp = union_sdf(base, stolp);
	stolp = substraction_sdf_smooth(stolp, punkt, r);
	
	return stolp;
}

float onion(float dist, float thickness) {
	return abs(dist) - thickness;
}

float building(vec3 p) {
	float base = onion(dist_box(p - vec3(0, 0, 4), vec3(20, 10, 5)), 0.5);
	float roof = dist_box(p - vec3(0, 0, 10), vec3(22, 12, 1));

	float door = dist_box(p - vec3(-20, 5, 2), vec3(1, 1.5, 2));
	float window = dist_box(p - vec3(-20, -7, 3), vec3(1, 2, 2));

	float coz = dist_box(p - vec3(-22, -7, 5.5), vec3(2.5, 2.5, 0.25));
	float stolp1 = dist_box(p - vec3(-24, -9.3, 2.75), vec3(0.2, 0.2, 2.75));
	float stolp2 = dist_box(p - vec3(-24, -4.7, 2.75), vec3(0.2, 0.2, 2.75));

	float dist = union_sdf(base, roof);
	dist = substraction_sdf(dist, door);
	dist = substraction_sdf(dist, window);
	dist = union_sdf(dist, coz);
	dist = union_sdf(dist, stolp1);
	dist = union_sdf(dist, stolp2);
	return dist;
}

float dist_lamp(vec3 p, float h, float r) {
	float dist = dist_segment(p, vec3(0, 0, 0), vec3(0, 0, h)) - r;
	dist = union_sdf(dist, dist_sphere(p - vec3(0, 0, h), r * 1.5));
	vec3 q = p;
	float k = 1.4;
	q.x *= k;
	float lamp = (dist_segment(q, vec3(0, 0, h), vec3(h * 0.15, 0, h * 1.04))) / k - r;
	float lamp_minus = dist_segment(q - vec3(0.2, 0, -0.3), vec3(0, 0, h), vec3(h * 0.15, 0, h * 1.04)) / k - r;
	lamp = substraction_sdf(lamp, lamp_minus);
	dist = union_sdf(dist, lamp);
	return dist;
}

// finaly logic code

float scene_distance(vec3 p) {	
	MODULE = 0;
	float module = 20.0;
	float dist = dist_lamp(rotated_point(p - vec3(-40, -12, 0), vec3(0, 0, -0.5 * sin(time))), 15, 0.35);
	dist = union_sdf(dist, building(p));
	float plane = dist_plane(p, vec3(0, 0, 1), 0);
	dist = union_sdf(dist, plane);

	return dist;
}

vec3 scene_normal(const vec3 p) {
	float d = scene_distance(p).x;
    vec2 e = vec2(DELTA, 0);
    vec3 n = d - vec3(scene_distance(p - e.xyy).x, scene_distance(p - e.yxy).x, scene_distance(p - e.yyx).x);
    return normalize(n);
}

vec4 intersection(const vec3 orig, const vec3 direction) {
	vec3 dir = normalize(direction);
	vec3 p = orig;
	float min_dist = inf;
	float inter;
	for (int i = 0; i < MAX_ITTERS; ++i) {
		inter = scene_distance(p);
		float dist = inter;
		min_dist = min(dist, min_dist);
		if (min_dist < MIN_DIST) {
			break;
		}
		p += dir * dist * STEP_COEF;
	}
	return vec4(p, inter);
}

float shadow(vec3 o, vec3 d, float mind, float maxd, float k) {
	d = normalize(d);
    for( float dist=mind; dist<maxd; )
    {
        float h = scene_distance(o + d*dist);
        if (h < 0.001)
            return 0.0;
        dist += h;
    }
    return 1.0;
}

vec3 phong_light(vec3 p, vec3 camera_pos, vec3 light_pos, vec3 color_diffuse, vec3 color_specular, float alpha, float intensity, float r1, float r2) {
	vec3 n = scene_normal(p);
	vec3 p_o = normalize(light_pos - p); // to light
	vec3 p_c = normalize(camera_pos - p); // to camera
	vec3 ref = normalize(reflect(-p_o, n)); // reflected light ray
	float dot_pn = dot(p_o, n);
	vec3 fix = n * dot(n, -p_o);
	float dot_pc = dot(-p_o + 2.0 * fix, p_c); // plz fix me
	if (dot_pn < 0.0) {
		return vec3(0, 0, 0);
	}

	vec3 ret;
	if (dot_pc < 0.0) {
		ret = intensity * color_diffuse * dot_pn;
	} else {
		ret = intensity * color_diffuse * dot_pn + color_specular * pow(dot_pc, alpha);
	}

	float l = length(light_pos - p);
	if (l > r1) {
		float k = 1 / (r2 - r1);
		return ret * max(0, (1 - pow(k * (l - r1), 0.75)));
	} else {
		return ret;
	}
}

vec3 victorious_march(const vec3 origin, const vec3 direction) {
	vec3 dir = normalize(direction);
	dir.z *= -1.0; // I just reverse Z axis cuz I want
	vec3 p = origin;
	vec3 color = vec3(0, 0, 0);

	float min_dist = inf;
	for (int i = 0; i < MAX_ITTERS; ++i) {
		float dist = scene_distance(p);
		min_dist = min(dist, min_dist);
		
		if (min_dist < MIN_DIST) {
			color = vec3(1, 1, 1);
		}
		
		p += dir * dist * STEP_COEF;

		if (min_dist > inf + 1.0) {
			break;
		}
	}

	if (min_dist < inf) {
		vec3 light_effect = AMBIENT;
		vec3 le;
		vec3 pos;
		vec3 clr;
		float r1;
		float r2;

		pos = vec3(100, 100, 100);
		clr = vec3(0.8, 0.2, 0.1);
		le = phong_light(moduled_point(p, MODULE), origin, pos, clr, clr * 0.5, 10, 1, 1000, 10000);
		light_effect += le * shadow(p, pos - p, 0.01, min(length(pos - p), 1000), 128);

		pos = vec3(0, 0, 8);
		clr = vec3(0.2, 0.5, 0.9);
		le = phong_light(moduled_point(p, MODULE), origin, pos, clr, clr * 0.5, 10, 4, 20, 40);
		light_effect += max(le * shadow(p, pos - p, 0.01, min(length(pos - p), 1000), 128), 0);

		pos = vec3(-21.1, 0, 8);
		clr = vec3(0, 1, 0);
		le = phong_light(moduled_point(p, MODULE), origin, pos, clr, clr * 0.5, 10, 1, 0, 15);
		//light_effect += le * shadow(p, pos - p, 0.05, min(length(pos - p), 1000), 128);

		pos = (vec3(-40, -12, 0) + vec3(-37.75, -10.77, 0))/2 + vec3(0, 0, 15.55);
		clr = vec3(1, 0.7, 0);
		le = phong_light(moduled_point(p, MODULE), origin, pos, clr, clr * 0.5, 10, 1, 17.5, 35);
		light_effect += le * shadow(p, pos - p, 0.01, min(length(pos - p), 1000), 128);

		color *= light_effect;
	}

	return color;
}
 

void main() {
	vec3 dx = ort1 * gl_FragCoord.x * w / res_x;
    vec3 dy = ort2 * gl_FragCoord.y * h / res_y;
	vec3 ray_origin = origin;
	vec3 ray_direction = normalize(left_upper - dx - dy - origin);
	vec3 color = victorious_march(ray_origin, ray_direction);
	gl_FragColor = vec4(color, 1);
}