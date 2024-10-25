cbuffer SimulationParams : register(b0)
{
    float3 Gravity;
    float DeltaTime;
};

RWStructuredBuffer<Particle> particles : register(u0);

[numthreads(32, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    if (index >= PARTICLE_COUNT)
        return;

    Particle p = particles[index];
    
    // Update particle position based on velocity and gravity
    p.Velocity.xyz += Gravity * DeltaTime;
    p.Position.xyz += p.Velocity.xyz * DeltaTime;

    // Write the updated particle back
    particles[index] = p;
}
