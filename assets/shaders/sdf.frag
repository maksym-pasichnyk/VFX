#version 450

layout(push_constant) uniform Globals {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 ViewProjectionMatrix;
    vec3 CameraPosition;

    ivec2 Resolution;
    float Time;

    mat4 ModelMatrix;
};

layout(location = 0) out vec4 out_color;

layout(location = 0) in struct {
    vec2 fragCoord;
} v_in;

void mainImage(out vec4 fragColor, in vec2 fragCoord);
void main() {
    mainImage(out_color, v_in.fragCoord);
}

/*---------------------*/

struct Ray {
    vec3 orig;
    vec3 dir;
};

struct Light {
    vec3 position;
    float intensity;
};

struct Material {
    vec4 albedo;
    vec3 diffuse_color;
    float specular_exponent;
    float refractive_index;
};

struct Sphere {
    vec3 center;
    float radius;
};

struct Box {
    vec3 center;
    vec3 size;
};

struct Plane {
    vec3 normal;
    vec3 center;
};

struct Capsule {
    vec3 start;
    vec3 up;
    vec2 size;
};

struct Cylinder {
    vec3 start;
    vec3 up;
    vec2 size;
};

struct Torus {
    vec3 center;
    vec2 radius;
};

struct RaycastHit {
    vec3 point;
    vec3 normal;
    float dist;
};

Sphere[] spheres = Sphere[](
    Sphere(vec3(0, 2, 0), 2.0f)
);

Capsule[] capsules = Capsule[] (
    Capsule(vec3(0, 0, 0), vec3(0, 1, 0), vec2(0.5f, 3))
);

Torus[] toruses = Torus[] (
    Torus(vec3(0, 2, 0), vec2(0.2f, 2.0f))
);

Box[] boxes = Box[] (
    Box(vec3(0, 2, 0), vec3(3, 1, 1))
);

Cylinder[] cylinders = Cylinder[] (
    Cylinder(vec3(0, 2, -3), normalize(vec3(0, 0, 1)), vec2(0.2f, 3))
);

Plane[] planes = Plane[](
    Plane(vec3(0, 1, 0), vec3(0, -5, 0))
);

Light[] lights = Light[](
    Light(vec3(-20, 20, 0), 0.5f),
    Light(vec3(0, 20, -20), 0.5f)
);

#define MAX_STEPS 100
#define MAX_DIST 1000.0f
#define SURFACE_DIST 1e-3f

vec3 offset(vec3 dir, vec3 normal, vec3 point, float offset) {
    if (dot(dir, normal) < 0.0f) {
        return point - normal * offset;
    }
    return point + normal * offset;
}

float sphereSDF(vec3 point, Sphere obj) {
    float r = obj.radius;
    vec3 C = obj.center;

    return length(point - C) - r;
}

float capsuleSDF(vec3 point, Capsule obj) {
    vec3 ab = obj.up * obj.size.y;
    vec3 start = obj.start + obj.up;

    vec3 ap = point - start;

    float r = obj.size.x;
    float t = clamp(dot(ab, ap) / dot(ab, ab), 0, 1);
    vec3 C = start + t * ab;

    return length(point - C) - r;
}

float cylinderSDF(vec3 point, Cylinder obj) {
    vec3 dir = obj.up * obj.size.y;
    vec3 start = obj.start + obj.up;

    vec3 ap = (point - start);

    float h = obj.size.y;
    float h2 = h * h;

    float r = obj.size.x;
    float t = dot(dir, ap) / h2;
    vec3 C = start + t * dir;
    float x = length(point - C) - r;
    float y = (abs(t - 0.5f) - 0.5f) * length(dir);
    float e = length(max(vec2(x, y), 0));
    return e + min(max(x, y), 0);
}

float torusSDF(vec3 point, Torus obj) {
    float R = obj.radius.y;
    float r = obj.radius.x;
    vec3 C = point - obj.center;
    float P = length(C.xz) - R;
    return length(vec2(P, C.y)) - r;
}

float boxSDF(vec3 point, Box obj) {
    vec3 C = abs(obj.center - point) - obj.size;
    float inside = min(max(C.x, max(C.y, C.z)), 0);
    float outside = length(max(C, 0));
    return inside + outside;
}

float smin(float a, float b, float k) {
    float h = max(k - abs(a - b), 0);
    return min(a, b) - h * h * 0.25 / k;
}

vec2 smin(vec2 a, vec2 b, float k) {
    float h = clamp(0.5f + (b.x - a.x) * 0.5f / k, 0, 1);
    return mix(b, a, h) - k * h * (1.0f - h);
}

float smax(float a, float b, float k) {
    float h = max(k - abs(a - b), 0);
    return max(a, b) - h * h * 0.25 / k;
}

float scene(vec3 point) {
    float d = 100000.0f;
    for (int i = 0; i < spheres.length(); i++) {
        d = smin(d, sphereSDF(point, spheres[i]), 0.5f);
    }
//    for (int i = 0; i < capsules.length(); i++) {
//        d = smin(d, capsuleSDF(point, capsules[i]), 0.5f);
//    }
//    for (int i = 0; i < toruses.length(); i++) {
//        d = smin(d, torusSDF(point, toruses[i]), 0.5f);
//    }
//    for (int i = 0; i < boxes.length(); i++) {
//        d = max(d, -boxSDF(point, boxes[i]));
//    }
//    for (int i = 0; i < cylinders.length(); i++) {
//        d = smin(d, cylinderSDF(point, cylinders[i]), 0.5f);
//    }

    return min(d, point.y);
}

vec3 getNormal(vec3 point) {
    vec2 e = vec2(SURFACE_DIST, 0);

    return normalize(
        scene(point) - vec3(
            scene(point - e.xyy),
            scene(point - e.yxy),
            scene(point - e.yyx)
        )
    );
}

float render(in Ray ray) {
    vec3 p = ray.orig;
    float depth = 0;
    for (int i = 0; depth < MAX_DIST && i < MAX_STEPS; i++) {
        float d = scene(p);
        depth += d;
        p += ray.dir * d;

        if (depth < SURFACE_DIST) {
            break;
        }
    }
    return depth;
}

vec2 getLight(vec3 point) {
    float diffuse = 0.0f;
    float specular = 0.0f;

    for (int i = 0; i < lights.length(); i++) {
        Light light = lights[i];
        vec3 light_dir = light.position - point;
        vec3 light_norm = normalize(light_dir);
        float light_dist = dot(light_dir, light_dir);

        vec3 normal = getNormal(point);

        float L = dot(light_norm, normal);
        if (L > 0.0f) {
            vec3 orig = offset(light_norm, normal, point, SURFACE_DIST * 2.0f);
            Ray ray = Ray(orig, light_norm);
            if (render(Ray(orig, light_norm)) >= length(light_dir)) {
                diffuse += L * light.intensity;
            }
        }
    }

    return vec2(diffuse, specular);
}

mat3 lookAt(vec3 origin, vec3 target, float roll) {
    vec3 rr = vec3(sin(roll), cos(roll), 0.0);
    vec3 ww = normalize(target - origin);
    vec3 uu = normalize(cross(ww, rr));
    vec3 vv = normalize(cross(uu, ww));

    return mat3(uu, vv, ww);
}

vec4 render(vec2 uv) {
    vec3 eye = vec3(-20, 20, -20);
    vec3 target = vec3(0, 0, 0);
    mat3 rot = lookAt(eye, target, 0.0f);

    vec3 rd = normalize(rot * vec3(uv, 2.0f));
    vec3 ro = eye;

    float depth = render(Ray(ro, rd));
    vec3 p = ro + rd * depth;

    vec2 light = getLight(p);
    vec3 col = vec3(light.x) * vec3(1.0f, 0.0f, 0.0f);

    return vec4(col, 1.);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 p0 = (fragCoord - vec2(Resolution.xy) * 0.5f) / vec2(Resolution.yy);

    fragColor = render(p0);
}