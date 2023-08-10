uniform mat4 model, view;
uniform sampler2D u_texture2d;
uniform vec3 u_coefficients_sh[9];
uniform bool u_textured = true;
uniform bool u_lit = false;
uniform bool u_matcaps = false;
uniform vec4 u_diffuse = vec4(1.0,1.0,1.0,1.0);

#ifdef RIM
in vec3 v_position;
#endif
in vec3 v_normal, v_normal_ws;
in vec2 v_texcoord;
in vec4 v_color;
out vec4 fragcolor;

{{include-shadowmap}}
in vec4 vpeye;
in vec4 vneye;
in vec4 sc;
vec4 shadowing() {
return shadowmap(vpeye, vneye, v_texcoord, sc);
}

void main() {
vec3 n = (v_normal);


vec4 lit = vec4(1.0, 1.0, 1.0, 1.0);
vec3 SHLightResult[9];
SHLightResult[0] =  0.282095f * u_coefficients_sh[0];
SHLightResult[1] = -0.488603f * u_coefficients_sh[1] * n.y;
SHLightResult[2] =  0.488603f * u_coefficients_sh[2] * n.z;
SHLightResult[3] = -0.488603f * u_coefficients_sh[3] * n.x;
SHLightResult[4] =  1.092548f * u_coefficients_sh[4] * n.x * n.y;
SHLightResult[5] = -1.092548f * u_coefficients_sh[5] * n.y * n.z;
SHLightResult[6] =  0.315392f * u_coefficients_sh[6] * (3.0f * n.z * n.z - 1.0f);
SHLightResult[7] = -1.092548f * u_coefficients_sh[7] * n.x * n.z;
SHLightResult[8] =  0.546274f * u_coefficients_sh[8] * (n.x * n.x - n.y * n.y);
vec3 result = vec3(0.0);
for (int i = 0; i < 9; ++i)
result += SHLightResult[i];
if( (result.x*result.x+result.y*result.y+result.z*result.z) > 0.0 ) lit = vec4(result, 1.0);




vec4 diffuse;
if(u_matcaps) {
    vec2 muv = vec2(view * vec4(v_normal_ws, 0))*0.5+vec2(0.5,0.5);
    diffuse = texture(u_texture2d, vec2(muv.x, 1.0-muv.y));
    } else if(u_textured) {
        diffuse = texture(u_texture2d, v_texcoord);
        } else {
            diffuse = u_diffuse;
        }
        
        
        fragcolor = diffuse * lit * shadowing();
        
        
        #ifdef RIM
        {vec3 n = normalize(mat3(M) * v_normal);
        vec3 p = (M * vec4(v_position,1.0)).xyz;
        vec3 v = normalize(-p);
        float rim = 1.0 - max(dot(v, n), 0.0);
        rim = smoothstep(1.0-0.01, 1.0, rim);
    fragcolor += vec4(0.0, 0.0, rim, 1.0);}
    #endif
    
}