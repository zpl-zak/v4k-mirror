#ifndef LIGHT_GLSL
#define LIGHT_GLSL

#define MAX_LIGHTS 16
#define MAX_SHADOW_LIGHTS 8
#define NUM_SHADOW_CASCADES 4

#include "brdf.glsl"

struct light_t {
    int type;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 pos;
    vec3 dir;
    float power;
    float radius;
    float innerCone;
    float outerCone;
    bool processed_shadows;

    // falloff
    float constant;
    float linear;
    float quadratic;

    // shadows
    float shadow_bias;
    float normal_bias;
    float min_variance;
    float variance_transition;
    float shadow_softness;
    float penumbra_size;
};

const int LIGHT_DIRECTIONAL = 0;
const int LIGHT_POINT = 1;
const int LIGHT_SPOT = 2;

uniform int u_num_lights;
uniform light_t u_lights[MAX_LIGHTS];

#ifdef FS_PASS
#include "shadowmap.glsl"
#endif

struct material_t {
    vec3 albedo;
    vec3 normal;
    vec3 F0;
    float roughness;
    float metallic;
    float alpha;
};

vec3 shading_light(light_t l, material_t m) {
    vec3 lightDir;
    float attenuation = 1.0;

    if (l.type == LIGHT_DIRECTIONAL) {
        lightDir = normalize(-l.dir);
    } else if (l.type == LIGHT_POINT || l.type == LIGHT_SPOT) {
        vec3 toLight = l.pos - v_position_ws;
        lightDir = normalize(toLight);
        float distance = length(toLight);
        
        /* fast-reject based on radius */
        if (l.radius != 0.0 && distance > l.radius) {
            return vec3(0,0,0);
        }
        attenuation = 1.0 / (l.constant + l.linear * distance + l.quadratic * (distance * distance));

        if (l.type == LIGHT_SPOT) {
            float angle = dot(l.dir, -lightDir);
            if (angle > l.innerCone) {
                float intensity = (angle-l.innerCone)/(l.outerCone-l.innerCone);
                attenuation *= clamp(intensity, 0.0, 1.0);
            } else {
                attenuation = 0.0;
            }
        }
    }

    // fast-rejection for faraway vertices
    if (attenuation <= 0.01) {
        return vec3(0,0,0);
    }

#ifdef SHADING_PBR
    vec3 radiance = l.diffuse * BOOST_LIGHTING;
    vec3 V = normalize( v_to_camera );
    vec3 N = m.normal;
    vec3 L = normalize( lightDir );
    vec3 H = normalize( V + L );

    vec3 F = fresnel_schlick( H, V, m.F0 );
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - m.metallic;

    // Premultiplied alpha applied to the diffuse component only
    kD *= m.alpha;

    float D = distribution_ggx( N, H, m.roughness );
    float G = geometry_smith( N, V, L, m.roughness );

    vec3 num = D * F * G;
    float denom = 4. * max( 0., dot( N, V ) ) * max( 0., dot( N, L ) );

    vec3 specular = kS * (num / max( 0.001, denom ));

    float NdotL = max( 0., dot( N, L ) );

    return ( kD * ( m.albedo / PI ) + specular ) * radiance * NdotL * attenuation;
#else
    vec3 n = normalize(v_normal_ws);

    float diffuse = max(dot(n, lightDir), 0.0);

    vec3 halfVec = normalize(lightDir + u_cam_dir);
    float specular = pow(max(dot(n, halfVec), 0.0), l.power);

    return (attenuation*l.ambient + diffuse*attenuation*l.diffuse + specular*attenuation*l.specular);
#endif
}

vec3 lighting(material_t m) {
    vec3 lit = vec3(0,0,0);
#ifndef SHADING_NONE
    for (int i=0; i<u_num_lights; i++) {
        vec3 lit_contrib = shading_light(u_lights[i], m);

#ifdef FS_PASS
        if (lit_contrib.xyz != vec3(0,0,0)) {
            lit += lit_contrib * shadowing(i).xyz;
        }
#endif
    }
#endif
    return lit;
}

#endif