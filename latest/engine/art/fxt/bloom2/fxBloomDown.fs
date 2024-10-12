void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 texel_size = 1.0 / textureSize(iChannel0, 0);
    float x = texel_size.x;
    float y = texel_size.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(iChannel0, vec2(uv.x - 2*x, uv.y + 2*y)).rgb;
    vec3 b = texture(iChannel0, vec2(uv.x,       uv.y + 2*y)).rgb;
    vec3 c = texture(iChannel0, vec2(uv.x + 2*x, uv.y + 2*y)).rgb;

    vec3 d = texture(iChannel0, vec2(uv.x - 2*x, uv.y)).rgb;
    vec3 e = texture(iChannel0, vec2(uv.x,       uv.y)).rgb;
    vec3 f = texture(iChannel0, vec2(uv.x + 2*x, uv.y)).rgb;

    vec3 g = texture(iChannel0, vec2(uv.x - 2*x, uv.y - 2*y)).rgb;
    vec3 h = texture(iChannel0, vec2(uv.x,       uv.y - 2*y)).rgb;
    vec3 i = texture(iChannel0, vec2(uv.x + 2*x, uv.y - 2*y)).rgb;

    vec3 j = texture(iChannel0, vec2(uv.x - x, uv.y + y)).rgb;
    vec3 k = texture(iChannel0, vec2(uv.x + x, uv.y + y)).rgb;
    vec3 l = texture(iChannel0, vec2(uv.x - x, uv.y - y)).rgb;
    vec3 m = texture(iChannel0, vec2(uv.x + x, uv.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    fragColor.rgb = e*0.125;
    fragColor.rgb = max(fragColor.rgb, 0.0001);
    fragColor.rgb += (a+c+g+i)*0.03125;
    fragColor.rgb += (b+d+f+h)*0.0625;
    fragColor.rgb += (j+k+l+m)*0.125;
    fragColor.a = 1.0;
}
