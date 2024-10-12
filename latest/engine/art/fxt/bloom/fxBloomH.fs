uniform float blur; /// set:16.0
uniform int steps; /// set:64

const float weights[64] = float[64](
    0.606531, 0.625479, 0.644389, 0.663223,
    0.681941, 0.700503, 0.718868, 0.736994,
    0.754840, 0.772363, 0.789522, 0.806274,
    0.822578, 0.838392, 0.853676, 0.868391,
    0.882497, 0.895957, 0.908734, 0.920793,
    0.932102, 0.942629, 0.952345, 0.961221,
    0.969233, 0.976358, 0.982575, 0.987867,
    0.992218, 0.995615, 0.998049, 0.999512,
    1.000000, 0.999512, 0.998049, 0.995615,
    0.992218, 0.987867, 0.982575, 0.976358,
    0.969233, 0.961221, 0.952345, 0.942629,
    0.932102, 0.920793, 0.908734, 0.895957,
    0.882497, 0.868391, 0.853676, 0.838392,
    0.822578, 0.806274, 0.789522, 0.772363,
    0.754840, 0.736994, 0.718868, 0.700503,
    0.681941, 0.663223, 0.644389, 0.625479
);

const float weightSum = 54.7568019644052;
const float stepSize = 0.25;
const int halfStep = 32;

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec3 color = vec3(0.0);
    vec2 uv = fragCoord.xy / iResolution.xy;
    float stepDir = blur / iResolution.x;

    for (int i = 0; i < steps; ++i) {
        int offset = i - halfStep;
        float x = float(offset) * stepSize;
        float weight = weights[i];
        vec2 offsetVec = vec2(x * stepDir, 0.0);
        color += texture(iChannel0, uv + offsetVec).rgb * weight;
    }

    color /= weightSum;
    fragColor = vec4(color, 1.0);
}
