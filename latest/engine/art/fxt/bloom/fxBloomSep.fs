uniform vec3 threshold; /// set:1.0,1.0,1.0
uniform float intensity; /// set:1.0
uniform vec3 tint; /// set:1.0,1.0,1.0

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec4 src = texture(iChannel0, uv);

    float brightness = dot(src.rgb, threshold);
    fragColor.rgb = (brightness > 1.0) ? src.rgb : vec3(0.0);
    fragColor.rgb *= intensity;
    fragColor.rgb *= tint;
    fragColor.a = 1.0;
}
