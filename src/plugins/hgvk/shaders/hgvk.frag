#version 450
layout(set = 0, binding = 0) uniform texture2D uTexture;
layout(set = 1, binding = 0) uniform sampler uSampler;
layout(push_constant) uniform Constants {
    uint flags;       // bit 0: colour-keyed texture, discard transparent texels
    float depthScale; // used by the vertex shader
} pc;

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 texel = texture(sampler2D(uTexture, uSampler), vUV);
    if ((pc.flags & 1u) != 0u && texel.a < 0.5)
        discard;
    outColor = texel * vColor;
}
