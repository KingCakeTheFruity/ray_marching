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

const float inf = 100000;
const float MIN_DIST = 0.001;
const float DELTA = 0.001;
const int MAX_ITTERS = 110;
const float STEP_COEF = 0.75;

float MODULE = 0;

const vec3 AMBIENT = vec3(0, 0, 0);

vec2 rotate(vec2 v, float a){
	return vec2(v.x * cos(a) - v.y * sin(a), v.x * sin(a) + v.y * cos(a));
}

vec3 rotate(vec3 v, vec2 ang){
    
    vec2 p = rotate(vec2(v.x, v.z), ang.y);
	v = vec3(p.x, v.y, p.y);
    
    v = vec3(rotate(vec2(v.x, v.y), ang.x), v.z);

    return v;
}

float smin(float a, float b, float k)
{
    float h = max( k-abs(a-b), 0.0 )/k;
    return min(a, b) - h*h*k*(1.0/4.0);
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

float union_sdf_smooth(float a, float b, float r) {
	vec2 u = max(vec2(r - a, r - b), vec2(0));
	return max(r, min(a, b)) - length(u);
}

float intersection_sdf_smooth(float a, float b, float r) {
	vec2 u = max(vec2(r + a, r + b), vec2(0));
	return min(-r, max(a, b)) + length(u);
}

float substraction_sdf_smooth(float a, float b, float r) {
	return intersection_sdf_smooth(a, -b, r);
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


vec3 repeated_point(vec3 p, float module, vec3 l) {
    vec3 q = p - module*clamp(round(p / module),-l,l);
    return q;
}

float dist_sphere(const vec3 p, vec3 pos, float r) {
	return distance(p, pos) - r;
}

float dist_box(vec3 p, vec3 pos, vec3 box) {
	vec3 q = p - pos;
    q = abs(q) - box;
	return length(max(q, 0));
}

float dist_cross(vec3 p, vec3 pos, float size) {
	float box1 = dist_box(p, pos, vec3(size, size, inf * 2.0));
	float box2 = dist_box(p, pos, vec3(size, inf * 2.0, size));
	float box3 = dist_box(p, pos, vec3(inf * 2.0, size, size));
	return union_sdf(union_sdf(box1, box2), box3);
}

float scene_distance(vec3 p) {	
	MODULE = 0;
	float module = 4.5;

	float s1 = dist_sphere(p, vec3(0, 0, 0), 2.0);
	// float box1 = dist_box(twisted_point(p, vec3(0, 0, 0.3)), vec3(0, 0, 0), vec3(1.5)) - 0.2;
	vec3 q = moduled_point(p, module);
	float box1 = dist_box(bended_point(repeated_point(p, module, vec3(3, 1, 0)), vec3(0, 0, 0.3 * sin(time)) * sin(time + p.y - q.y) * sin(time + p.x - q.x)), vec3(0, 0, 0.2 * sin(time + p.x - q.x) * sin(time + p.y - q.y)), vec3(2, 2, 0.3)) - 0.1;

	float grid = dist_box(moduled_point(p, 0.2), vec3(0, 0, 0), vec3(0.1));
	float dist = union_sdf_smooth(s1, box1, 0.2);

	return box1;
}

vec3 scene_normal(const vec3 p) {
	float d = scene_distance(p);
    vec2 e = vec2(DELTA, 0);
    vec3 n = d - vec3(scene_distance(p - e.xyy), scene_distance(p - e.yxy), scene_distance(p - e.yyx));
    return normalize(n);
}

vec3 intersection(const vec3 orig, const vec3 direction) {
	vec3 dir = normalize(direction);
	vec3 p = orig;
	float min_dist = inf;
	for (int i = 0; i < MAX_ITTERS; ++i) {
		float dist = scene_distance(p);
		min_dist = min(dist, min_dist);
		if (min_dist < MIN_DIST / 100.0) {
			break;
		}
		p += dir * dist * STEP_COEF;
	}
	return p;
}

vec3 phong_light(vec3 p, vec3 camera_pos, vec3 light_pos, vec3 color_diffuse, vec3 color_specular, float alpha, float intensity) {
	vec3 n = scene_normal(p);
	vec3 p_o = normalize(light_pos - p); // to light
	vec3 p_c = normalize(camera_pos - p); // to camera
	vec3 ref = normalize(reflect(-p_o, n)); // reflected light ray
	float dot_pn = dot(p_o, n);
	float dot_pc = dot(p_c, ref);
	if (dot_pn < 0.0 || length(intersection(p, p_o) - p) < length(p_o)) {
		return vec3(0, 0, 0);
	}
	if (dot_pc < 0.0) {
		return intensity * color_diffuse * dot_pn;
	} else {
		return intensity * color_diffuse * dot_pn + color_specular * pow(dot_pc, alpha);
	}
}

vec3 victorious_march(const vec3 origin, const vec3 direction) {
	vec3 dir = normalize(direction);
	dir.z *= -1; // I just reverse Z axis cuz I want
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
		vec3 n = scene_normal(p);
		p = p + n * MIN_DIST * (1.0 + DELTA);
		for (int i = 0; i <= lights_count; ++i) {
			light_effect += phong_light(moduled_point(p, MODULE), origin, light_o[i], light_color[i], light_color[i] * 0.5, 10, light_intensity[i]);
		}
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