void mainImage( out vec4 fragColor, in vec2 fragCoord );
void main() {
    mainImage(fragColor, texcoord.xy * iResolution);
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/2.2));
}