uniform float u_gamma;
out vec4 color;

void main(void) {
    vec2 uv = TEXCOORD.st;
    vec4 src = texture2D(iChannel0, uv);
    color = vec4( pow(src.xyz, vec3(u_gamma)), src.a); // gamma correction
}
