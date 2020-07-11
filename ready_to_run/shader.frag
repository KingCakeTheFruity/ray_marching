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
const int MAX_ITTERS = 100;
const float STEP_COEF = 1;

const float MODULE = 10;

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

float intersect_sdf(float dist_first, float dist_second) {
	return max(dist_first, dist_second);
}

float union_sdf(float dist_first, float dist_second, float smoothness) {
	return smin(dist_first, dist_second, smoothness);
}

float difference_sdf(float dist_first, float dist_second) {
	return max(dist_first, -dist_second);
}

vec3 moduled_point(vec3 p, float module) {
	if (module == 0.0) {
		return p;
	}
	return mod(p + module / 2.0, module) - module / 2.0;
}

float dist_sphere(const vec3 point, int i) {
	return distance(point, sph_o[i]) - sph_r[i];
}

float dist_box(vec3 p, vec3 pos, vec3 box) {
	vec3 q = p - pos;
    q = abs(q) - box;
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float dist_box_bended(vec3 p, vec3 pos, vec3 box, vec2 angle) {
	p = rotate(p, angle);
	return dist_box(p, pos, box);
}

float dist_sphere_triged (const vec3 point, int i) {
	vec3 p = point;
	float amp = 0.1;
	float cord = 10.0;
	float time_coef = 1.0;
	p.x = point.x + amp * sin(point.x * cord + time * time_coef);
	p.y = point.y + amp * sin(point.y * cord + time * time_coef);
	p.z = point.z + amp * sin(point.z * cord + time * time_coef);
	float dist = distance(p, sph_o[i]) - sph_r[i];
	return dist;
}

float scene_distance_standart(const vec3 point) {
	float dist = inf;
	for (int i = 0; i <= spheres_count; ++i) {
		dist = min(dist, dist_sphere(point, i));
	}
	return dist;
}

float scene_distance(vec3 point) {
	int mode = 0;
	if (mode > 0) {
		return scene_distance_standart(point);
	}
	
	float module = MODULE;
	float s1 = dist_sphere(point, 0);
	float s2 = dist_sphere(point, 1);
	float s3 = dist_sphere(point, 2);
	float box1 = dist_box_bended(point, vec3(0, 0, 5), vec3(1, 1, 7), vec2(point.z, 0) * sin(time / 2));
	float box2 = dist_box_bended(point, vec3(0, 0, 0), vec3(1, 10, 1), vec2(0, point.y) * sin(time / 3));
	//float s2 = dist_inf_spheres(point, 1, module);
	//float s3 = dist_inf_spheres(point, 2, module);
	//float s4 = dist_inf_spheres(point, 3, module);
	//float s5 = dist_inf_spheres(point, 4, module);
	//float box = dist_inf_boxes(point, vec3(0, 0, 0), 3.0, module);
	
	float ret = union_sdf(box1, s1, 0.5);
	ret = union_sdf(ret, box2, 0.5);
	ret = intersect_sdf(ret, s2);
	ret = union_sdf(ret, s3, 1);

	return ret; 
}

vec3 scene_normal(const vec3 point) {
	float d = scene_distance(point);
    vec2 e = vec2(DELTA, 0);
    vec3 n = d - vec3(scene_distance(point - e.xyy), scene_distance(point - e.yxy), scene_distance(point - e.yyx));
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