uniform sampler2D u_texture_y;
uniform sampler2D u_texture_cb;
uniform sampler2D u_texture_cr;
uniform float u_gamma;

in vec2 uv;
out vec4 fragcolor;

void main() {
    float y = texture(u_texture_y, uv).r;
    float cb = texture(u_texture_cb, uv).r;
    float cr = texture(u_texture_cr, uv).r;
    
    const mat4 to_rgb = mat4(
    1.0000, 1.0000, 1.0000, 0.0000,
    0.0000, -0.3441, 1.7720, 0.0000,
    1.4020, -0.7141, 0.0000, 0.0000,
    -0.7010, 0.5291, -0.8860, 1.0000
    );
    vec4 texel = to_rgb * vec4(y, cb, cr, 1.0);
    
    
    texel.rgb = pow(texel.rgb, vec3(1.0 / u_gamma));
    
    
    if(false) { float saturation = 2.0; const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(texel.rgb, W));
texel.rgb = mix(intensity, texel.rgb, saturation); }

fragcolor = vec4(texel.rgb, 1.0);
}