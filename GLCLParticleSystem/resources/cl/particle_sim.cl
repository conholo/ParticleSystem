
#define SPHERE_COUNT 9
#define PI 3.14159265359

typedef struct simulation_bounds
{
	float4 Center;
	float4 MinExtent;
	float4 MaxExtent;
} simulation_bounds;

float random(float3 v)
{
	float r;
	float d = dot(v, (float3)(12.9898, 78.233, 1234.343));
	float s = sin(d) * 43758.5453f;
	float f = fract(s, &r);
	return f;
}

bool IsInsideSphere(float4 p, float4 s)
{
	float r = fast_length(p.xyz - s.xyz);
	return r < s.w;
}

bool InColumn(float4 p, simulation_bounds* bounds)
{
	long difference = (long)(bounds->MaxExtent.x - bounds->MinExtent.x);
	float boundsSize = (float)abs(difference);

	float3 columnSize = (float3)(boundsSize / 4.0f, boundsSize / 2.0f, boundsSize / 4.0f);

	float3 minColumnExtents = bounds->Center.xyz - columnSize / 2.0f;
	float3 maxColumnExtents = bounds->Center.xyz + columnSize / 2.0f;

	bool inX = p.x >= minColumnExtents.x && p.x <= maxColumnExtents.x;
	bool inY = p.y >= minColumnExtents.y && p.y <= maxColumnExtents.y;
	bool inZ = p.z >= minColumnExtents.z && p.z <= maxColumnExtents.z;

	return inX && inY && inZ;
}

float4 ResolveCollision(float3 i, float3 n)
{
	i = fast_normalize(i);
	n = fast_normalize(n);
	float3 r = i - n * (float3)(2.0 * dot(i, n));
	return (float4)(r.xyz, 0.0);
}

float4 UpdateVelocity(float4 p, float4 v, simulation_bounds* bounds, float4* spheresBuffer)
{
	if (InColumn(p, bounds) && v.y < 0.0f)
		return v;

	for (int i = 0; i < SPHERE_COUNT; i++)
		if (IsInsideSphere(p, spheresBuffer[i]))
			return ResolveCollision(v.xyz, (float3)(p.xyz - spheresBuffer[i].xyz));

	if (p.y < bounds->MinExtent.y)
		return ResolveCollision(v.xyz, (float3)(0.0, 1.0, 0.0));
	if (p.y > bounds->MaxExtent.y)
		return ResolveCollision(v.xyz, (float3)(0.0, -1.0, 0.0));
	if (p.x < bounds->MinExtent.x)
		return ResolveCollision(v.xyz, (float3)(1.0, 0.0, 0.0));
	if (p.x > bounds->MaxExtent.x)
		return ResolveCollision(v.xyz, (float3)(-1.0, 0.0, 0.0));
	if (p.z < bounds->MinExtent.z)
		return ResolveCollision(v.xyz, (float3)(0.0, 0.0, 1.0));
	if (p.z > bounds->MaxExtent.z)
		return ResolveCollision(v.xyz, (float3)(0.0, 0.0, -1.0));

	return v;
}

float3 ColorFromAngle(float a)
{
	return (float3)(256 * cos(a), 256 * cos(a + 120), 256 * cos(a - 120));

}


float Remap01(float low, float high, float t)
{
	return (t - low) / (high - low);
}


kernel void ParticleSimulation(global float4* positionBuffer, global float4* velocityBuffer, global float4* colorBuffer, global simulation_bounds* bounds, global float4* spheresBuffer, global float* time)
{
	constant float4 G = (float4) (0., -9.8 * 4, 0., 0.);
	constant float  DT = 0.00125;
	int gid = get_global_id(0);

	float4 p = positionBuffer[gid];
	float4 v = velocityBuffer[gid];
	float4 c = colorBuffer[gid];

	float4 pp = p + v * DT + G * (float4)(0.5 * DT * DT);
	pp.w = 1.0;
	float4 vp = UpdateVelocity(pp, v + G * DT, bounds, spheresBuffer);
	vp.w = 0.0;

	pp = p + vp * DT + G * (float4)(0.5 * DT * DT);

	long size = abs((long)(bounds->MaxExtent.x - bounds->MinExtent.x));

	float heightPercent = Remap01(bounds->MinExtent.y, bounds->MaxExtent.y, p.y);

	float3 xyzPercent = (float3)(
		Remap01(bounds->MinExtent.x, bounds->MaxExtent.x, p.x), 
		Remap01(bounds->MinExtent.y, bounds->MaxExtent.y, p.y), 
		Remap01(bounds->MinExtent.z, bounds->MaxExtent.z, p.z)
	);

	float3 randomOverTime = (float3)(0.5f, 0.5f, 0.5f) + (float3)(0.5f, 0.5f, 0.5f) * cos((float3)(*time, *time, *time) + xyzPercent + (float3)(0, 2, 4));
	float3 color = mix(randomOverTime, (float3)(0.0, 1.0, 0.0), heightPercent);
	
	positionBuffer[gid] = pp;
	velocityBuffer[gid] = vp;
	colorBuffer[gid] = (float4)(color.x, color.y, color.z, 1.0f);
}

kernel void ApplyPulse(global float4* positionBuffer, global float4* velocityBuffer, global simulation_bounds* bounds)
{
	constant float4 G = (float4) (0., -9.8 * 4, 0., 0.);
	constant float  DT = 0.00125;
	int gid = get_global_id(0);

	long size = abs((long)(bounds->MaxExtent.x - bounds->MinExtent.x));
	float3 bottomCenter = (float3)(0.0f, bounds->MinExtent.y, 0.0f);

	float4 p = positionBuffer[gid];

	float maxForce = 50.0f;
	float yEffect = pow((bounds->MaxExtent.y - p.y) / (float)size, 2.0f);
	float forcePercent = pow(((float)(size) - length(p.xyz - bottomCenter)) / (float)(size), 2.0f);

	float r = random(p.xyz);
	float4 vel = (float4)(0.0f, 1.0f, 0.0f, 0.0f) * maxForce * forcePercent * yEffect * r;
	velocityBuffer[gid] += vel;
}

