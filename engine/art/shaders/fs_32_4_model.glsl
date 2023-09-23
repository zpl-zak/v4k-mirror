uniform mat4 model, view;
uniform sampler2D u_texture2d;
uniform vec3 u_coefficients_sh[9];
uniform bool u_textured = true;
uniform bool u_lit = false;
uniform bool u_matcaps = false;
uniform vec4 u_diffuse = vec4(1.0,1.0,1.0,1.0);

in vec3 v_position;
#ifdef RIM
uniform mat4 M; // RIM
uniform vec3 u_rimcolor = vec3(0.2,0.2,0.2);
uniform vec3 u_rimrange = vec3(0.11,0.98,0.5);
uniform vec3 u_rimpivot = vec3(0,0,0);
uniform bool u_rimambient = true;
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

uniform int u_num_lights;

struct light_t {
    int type;
    vec3 color;
    vec3 pos;
    vec3 dir;
    float radius;
};

#define MAX_LIGHTS 16
const int LIGHT_DIRECTIONAL = 0;
const int LIGHT_POINT = 1;
const int LIGHT_SPOT = 2;

uniform light_t u_lights[MAX_LIGHTS];

vec3 calculate_light(light_t l, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightColor = l.color;

    vec3 lightDir;
    float attenuation = 1.0;

    if (l.type == LIGHT_DIRECTIONAL) {
        lightDir = normalize(-l.dir);
    } else if (l.type == LIGHT_POINT) {
        vec3 toLight = fragPos - l.pos;
        lightDir = normalize(toLight);
        float distance = length(toLight);
        float factor = distance / l.radius;
        attenuation = clamp(1.0 - factor, 0.0, 1.0);
    }

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // vec3 specular = spec * lightColor;

    return (diffuse /* + specular */) * l.color;
}

void main() {
    vec3 n = /*normalize*/(v_normal);

    vec4 lit = vec4(1.0, 1.0, 1.0, 1.0);
    // SH lighting
    {
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
    }

    // analytical lights (phong shading)
    // @todo: support more shading models (blinn-phong, ue4 brdf, ...)
    // for (int i=0; i<u_num_lights; i++) {
    //     lit += vec4(calculate_light(u_lights[i], n, v_position, /* @todo: push vdeye */ vec3(1.0,1.0,1.0)), 0.0);
    // }

    // base
    vec4 diffuse;
 
    if(u_matcaps) {
        vec2 muv = vec2(view * vec4(v_normal_ws, 0))*0.5+vec2(0.5,0.5); // normal (model space) to view space
        diffuse = texture(u_texture2d, vec2(muv.x, 1.0-muv.y));
    } else if(u_textured) {
        diffuse = texture(u_texture2d, v_texcoord);
        } else {
    diffuse = u_diffuse; // * v_color;
    }
    
    // lighting mix
    fragcolor = diffuse * lit * shadowing();
    
    // rimlight
    #ifdef RIM
    {vec3 n = normalize(mat3(M) * v_normal);  // convert normal to view space
    vec3 p = (M * vec4(v_position,1.0)).xyz; // convert position to view space
    vec3 v = vec3(0,-1,0);
    if (!u_rimambient) {
        v = normalize(u_rimpivot-p);
    }
    float rim = 1.0 - max(dot(v,n), 0.0);
    vec3 col = u_rimcolor*(pow(smoothstep(1.0-u_rimrange.x,u_rimrange.y,rim), u_rimrange.z));
    fragcolor += vec4(col, 1.0);}
    #endif
}