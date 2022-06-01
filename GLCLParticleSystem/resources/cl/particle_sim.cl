
typedef struct simulation_bounds
{
	float4 Center;
	float4 MinExtent;
	float4 MaxExtent;
} simulation_bounds;

bool IsInsideSphere(float4 p, float4 s)
{
	float r = fast_length(p.xyz - s.xyz);
	return r < s.w;
}

float4 ResolveCollision(float3 i, float3 n)
{
	i = fast_normalize(i);
	n = fast_normalize(n);
	float3 r = i - n * (float3)(2.0 * dot(i, n));
	return (float4)(r.xyz, 0.0);
}

float4 UpdateVelocity(float4 p, float4 v, simulation_bounds* bounds, float4 sphere)
{
	if (IsInsideSphere(p, sphere))
		return ResolveCollision(v.xyz, (float3)(p.xyz - sphere.xyz));
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
	if (p.z > bounds->MaxExtent.x)
		return ResolveCollision(v.xyz, (float3)(0.0, 0.0, -1.0));

	return v;
}

kernel void ParticleSimulation(global float4* positionBuffer, global float4* velocityBuffer, global float4* colorBuffer, global simulation_bounds* bounds)
{
	constant float4 G = (float4) (0., -9.8, 0., 0.);
	constant float  DT = 0.001;
	long difference = (long)(bounds->MaxExtent.x - bounds->MinExtent.x);
	float radius = (float)abs(difference) / 4.0f;
	float4 sphere = (float4)(bounds->Center.xyz, radius);

	int gid = get_global_id(0);

	float4 p = positionBuffer[gid];
	float4 v = velocityBuffer[gid];
	float4 c = colorBuffer[gid];


	float4 pp = p + v * DT + G * (float4)(0.5 * DT * DT);
	pp.w = 1.0;
	float4 vp = UpdateVelocity(pp, v + G * DT, bounds, sphere);
	vp.w = 0.0;

	pp = p + vp * DT + G * (float4)(0.5 * DT * DT);

	positionBuffer[gid] = pp;
	velocityBuffer[gid] = vp;
}
