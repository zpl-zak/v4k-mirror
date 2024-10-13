uniform float strength; /// set:0.04

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    fragColor.rgb = texture(iChannel0, uv).rgb * strength;
    fragColor.a = 1.0;
}
