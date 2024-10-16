uniform int thickness = 2;
uniform vec4 border_color = vec4(1,1,0,1);

void main() {
    vec4 texel = texture(iChannel0, uv);
    float outline = 0.0;
    if( texel.a == 0.0 ) {
        for( int x = -thickness; x <= thickness; x++ ) {
            for( int y = -thickness;y <= thickness; y++ ) {
                float sampleVal = texture(iChannel0, uv+vec2(float(x)/iWidth, float(y)/iHeight)).a;
                if( sampleVal > 0.0 ) {
                    outline = 1.0;
                }
            }
        }
    }

    FRAGCOLOR = vec4(border_color.rgb, outline * border_color.a); // mix(texel, border_color, outline * border_color.a);
}