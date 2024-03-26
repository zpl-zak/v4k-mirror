uniform samplerCube u_cubemap;


in vec3 v_direction;
out vec4 fragcolor;


void main() {
    fragcolor = vec4(texture(u_cubemap, v_direction).rgb, 1.0);
    fragcolor.rgb = pow(fragcolor.rgb, vec3(1.0/2.2));
}