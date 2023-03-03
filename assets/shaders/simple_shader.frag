#version 450

#define COUNT_VOXELS 8
#define COUNT_STEPS 32

layout(location = 0) in vec3 fragVertexColor;
layout(location = 1) in vec3 fragOrigin;
layout(location = 2) in vec3 fragDirection;

layout(location=0) out vec4 fragOutputColor;

layout(push_constant) uniform SceneConstants {
	mat4x4 ProjectionMatrix;
	mat4x4 WorldToCameraMatrix;
	mat4x4 ViewProjectionMatrix;
	mat4x4 InverseViewProjectionMatrix;
	uint   Volume[16];
	vec3   CameraPosition;
};

vec3 intersectAABB(vec3 rayPos, vec3 rayDir, vec3 aabbMin, vec3 aabbMax) {
	vec3 tMin = (aabbMin - rayPos) / rayDir;
	vec3 tMax = (aabbMax - rayPos) / rayDir;
	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
	return vec3(tNear, tFar, tFar - tNear);
}

void main () {
	vec3 rd = fragDirection;
	vec3 ro = CameraPosition;

	ro = ro + rd * (max(0, intersectAABB(ro, rd, vec3(-0.5), vec3(+0.5)).x));
	ro = (ro + 0.5) * COUNT_VOXELS;

	ivec3 mapPos = clamp(ivec3(floor(ro)), ivec3(0), ivec3(COUNT_VOXELS - 1));
	vec3 deltaDist = abs(vec3(length(rd)) / rd);
	ivec3 rayStep = ivec3(sign(rd));
	vec3 sideDist = (sign(rd) * (vec3(mapPos) - ro) + (sign(rd) * 0.5) + 0.5) * deltaDist;

	for (int i = 0; i < COUNT_STEPS; i++) {
		uint index = mapPos.x + mapPos.y * COUNT_VOXELS + mapPos.z * COUNT_VOXELS * COUNT_VOXELS;
		if ((Volume[index / 32] & (1u << (index % 32))) != 0) {
			fragOutputColor = vec4(1.0F);
			return;
		}
		bvec3 mask = lessThanEqual(sideDist.xyz, min(sideDist.yzx, sideDist.zxy));

		sideDist += vec3(mask) * deltaDist;
		mapPos += ivec3(vec3(mask)) * rayStep;
		if (clamp(mapPos, ivec3(0), ivec3(COUNT_VOXELS - 1)) != mapPos) {
			break;
		}
	}
	fragOutputColor = vec4(0.0F, 0, 0, 1);
}