#version 450
// Vertices arrive from the sim already projected to 640x480 screen pixels, with
// rhw = 1/z. Putting w = 1/rhw back into the clip position makes the hardware
// interpolate colour and texture coordinates perspective-correctly, exactly as
// Direct3D does for pre-transformed (TL) vertices.
// Depth is 1/z (inIz) scaled to 0..1, larger = nearer. 1/z is linear in screen
// space and z/w is interpolated linearly, so writing z = depth * w gives an
// exact W-buffer. rhw <= 0 marks a screen-space vertex (w = 1); inIz <= 0 is
// the farthest depth.
layout(location = 0) in vec3 inPos; // x, y in pixels, rhw
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in float inIz;

layout(push_constant) uniform Constants {
    uint flags;       // used by the fragment shader
    float depthScale; // inIz * depthScale = depth in 0..1
} pc;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vUV;

void main()
{
    float w = 1.0;
    float depth = 0.0;
    vec2 ndc = inPos.xy * vec2(2.0 / 640.0, 2.0 / 480.0) - 1.0;

    if (inPos.z > 0.0)
        w = 1.0 / inPos.z;
    if (inIz > 0.0)
        depth = clamp(inIz * pc.depthScale, 0.0, 1.0);
    gl_Position = vec4(ndc * w, depth * w, w);
    gl_PointSize = 1.0;
    vColor = inColor;
    vUV = inUV;
}
