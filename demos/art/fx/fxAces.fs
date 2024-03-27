out vec4 color;

void main(void) {
    vec2 uv = TEXCOORD.st;
    vec3 src = texture2D(iChannel0, uv).rgb;
    vec3 x = src;

    // aces film (CC0, src: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/)
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    src = clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
    color = vec4(src, 1.0);
}
