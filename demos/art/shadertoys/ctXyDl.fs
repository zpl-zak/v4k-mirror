float sdSphere(vec3 p, float r)
{
    return length(p) - r;
}

float sdCapsule(vec3 p, float h, float r)
{
    return length(vec3(p.x, max(0.0, abs(p.y) - 0.5*h), p.z)) - r;
}

float sdCylinder( vec3 p, float h, float r )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(r,h);
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}


struct Scene
{
    vec2 rot;
    vec2 stre;
};

mat2 rot2(float theta)
{
    float c = cos(theta);
    float s = sin(theta);
    return mat2(
        c, -s,
        s, c
    );
}

vec3 apply_rot(vec3 p, vec2 rot)
{
    p.xy *= rot2(rot.x);
    p.yz *= rot2(rot.y);
    return p;
}

float smax(float a, float b, float k)
{
    float dist = abs(a - b);
    float res = max(a, b);
    if (dist < k)
    {
        res += (1.0/(4.0*k))*(k-dist)*(k-dist);
    }
    return res;
}

float smin(float a, float b, float k)
{
    float dist = abs(a - b);
    float res = min(a, b);
    if (dist < k)
    {
        res -= (1.0/(4.0*k))*(k-dist)*(k-dist);
    }
    return res;
}

float map_shell(Scene scene, vec3 p)
{
    p = apply_rot(p, scene.rot);
    float dist = sdCapsule(p-vec3(0,-0.4,0), 1.3, 0.43);
    dist = smin(dist, sdCylinder(p-vec3(0,0.6,0), 0.6, 0.12), 0.03);
    dist = smax(dist, -p.y-1.1, 0.02);
    return dist;
}

vec3 calc_normal_shell(Scene scene, vec3 p)
{
    vec2 e = vec2(0, 0.0001);
    return normalize(vec3(map_shell(scene, p+e.yxx),
                          map_shell(scene, p+e.xyx),
                          map_shell(scene, p+e.xxy)) - 
                          vec3(map_shell(scene, p)));
}

float map_body(Scene scene, vec3 p)
{
    float dist = map_shell(scene, p)+0.04;
    
    vec3 surfaceN = vec3(0,1,0);
    // surface wobble
    surfaceN.yz *= rot2(scene.stre.y*sin(10.0*iTime));
    surfaceN.xy *= rot2(scene.stre.x*sin(10.0*iTime));
    // travelling waves
    float perturb = 0.1*(scene.stre.x*sin(4.0*(p.x+sign(scene.stre.x)*5.0*iTime)) + 
                         scene.stre.y*sin(4.0*(p.z+sign(scene.stre.y)*5.0*iTime)));
    // TODO: SDF is completely ruined if I remove the *0.1 here, why?
    //       does this screw up the SDF that badly?
    dist = 0.1*max(dist, dot(p,surfaceN)-(0.0+perturb));
    return dist;
}

vec3 calc_normal_body(Scene scene, vec3 p)
{
    vec2 e = vec2(0, 0.0001);
    return normalize(vec3(map_body(scene,p+e.yxx),
                          map_body(scene,p+e.xyx),
                          map_body(scene,p+e.xxy)) - 
                          vec3(map_body(scene,p)));
}

vec3 render(Scene scene, vec2 uv)
{
    float cam_angle = 0.0 * 6.28;
    vec3 ro = 2.0*vec3(sin(cam_angle), 0.2, -cos(cam_angle));
    vec3 at = vec3(0);
    vec3 cam_z = normalize(at - ro);
    vec3 cam_x = normalize(cross(vec3(0, 1, 0), cam_z));
    vec3 cam_y = cross(cam_z, cam_x);
    vec3 rd = normalize(uv.x * cam_x + uv.y * cam_y + 1.3 * cam_z);
    
    bool hit = false;
    float t = 0.0;
    for (int i = 0; i < 256; ++i)
    {
        vec3 p = ro + t * rd;
        float dist = 0.1*map_shell(scene, p);
        if (dist < 0.001)
        {
            hit = true;
            break;
        }
        t += dist;
    }
    
    bool hit_inner = false;
    float t_inner = 0.0;
    for (int i = 0; i < 256; ++i)
    {
        vec3 p = ro + t_inner * rd;
        float dist = map_body(scene, p);
        if (dist < 0.001)
        {
            hit_inner = true;
            break;
        }
        t_inner += dist;
    }

    vec3 glass_f0 = vec3(0.5);
    vec3 water_f0 = vec3(0.3);
    vec3 keyLightCol = 1.5*vec3(1);//vec3(0.3, 0.5, 1.1);
    vec3 keyLightDir = normalize(vec3(0.0,0.9,0.5));
    vec3 waterDifCol = 1.3*vec3(0.3,0.6,0.8);
    
    vec3 bgCol = texture(iChannel0, rd).rgb;
    bgCol = bgCol * bgCol; // gamma -> linear
  
    vec3 col = bgCol;
    vec3 glass_fre = vec3(1);
    if (hit)
    {
        vec3 p = ro + t * rd;
        vec3 n = calc_normal_shell(scene, p);
        vec3 l = rd - 2.0 * dot(rd, n) * n;
        glass_fre = glass_f0 + (vec3(1) - glass_f0) * pow(1.0 - dot(n, l), 5.0);
        
        {
            float keyLightCos = dot(keyLightDir, l);

            vec3 keyLight = texture(iChannel1, l).rgb;
            vec3 spe = keyLight * glass_fre;
            col = spe;
        }
    }
    
    if (hit_inner)
    {
        vec3 p = ro + t_inner * rd;
        vec3 n = calc_normal_body(scene, p);
        vec3 l = rd - 2.0 * dot(rd, n) * n;
        vec3 fre = water_f0 + (vec3(1) - water_f0) * pow(1.0 - dot(n, l), 5.0);
        
        {
            float keyLightCos = dot(keyLightDir, l);

            vec3 keyLight = texture(iChannel1, l).rgb;
            vec3 spe = keyLight * fre;
            vec3 dif = waterDifCol * keyLightCol * (0.05 + 0.3 * (0.5 + 0.5 * keyLightCos));
            col = mix(col, spe + dif, 1.0-glass_fre);
        }
    }
    else
    {
        col = mix(col, bgCol,1.0-glass_fre);
    }
    return col;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    uv = 2.0 * uv - 1.0;
    uv.x *= iResolution.x / iResolution.y;
    
#define SIM1_DATA_IDX 0
#define SIM2_DATA_IDX 1
#define DATA_STRIDE 2
    // simulation data stored in double-buffered packs of floats for persistent storage
    int currPackIdx = iFrame & 1;
    int nextPackIdx = (iFrame + 1) & 1;
    vec4 sim1Data = texelFetch(iChannel2, ivec2(DATA_STRIDE*currPackIdx + SIM1_DATA_IDX, 0), 0);
    vec4 sim2Data = texelFetch(iChannel2, ivec2(DATA_STRIDE*currPackIdx + SIM2_DATA_IDX, 0), 0);

    vec2 prevMouseCoord = sim1Data.xy;
    vec2 prevRot = sim1Data.zw;
    vec2 prevStre = sim2Data.xy;
    
    // reference: https://shadertoyunofficial.wordpress.com/2016/07/20/special-shadertoy-features/
    bool mouseDown = iMouse.z > 0. && iMouse.w < 0.;
        
    // compute current rotation
    vec2 deltaRot = vec2(0);
    if (iMouse.x > 0. && mouseDown)
        deltaRot.x = (iMouse.x - prevMouseCoord.x) / 800.0 * 6.28;
    if (iMouse.y > 0. && mouseDown)
        deltaRot.y -= (iMouse.y - prevMouseCoord.y) / 450.0 * 6.28;
    vec2 currRot = prevRot + deltaRot;
    // dampening oscillation
    vec2 stre = 0.3*abs(deltaRot) + 0.987*prevStre;
    stre = min(vec2(0.5), stre);

    // special routine for data pixels (not color pixels)
    ivec2 iFragCoord = ivec2(fragCoord);
    int nextPackBegin = nextPackIdx * DATA_STRIDE;
    int nextPackEnd = nextPackBegin + DATA_STRIDE;
    if (iFragCoord.x >= nextPackBegin && iFragCoord.y < nextPackEnd && iFragCoord.y == 0)
    {
        int dataIdx = iFragCoord.x - nextPackBegin;
        if (dataIdx == SIM1_DATA_IDX)
            fragColor = vec4(iMouse.xy, currRot);
        if (dataIdx == SIM2_DATA_IDX)
            fragColor = vec4(stre, 0, 0);
        return;
    }
    
    Scene scene;
    scene.rot = currRot;
    scene.stre = stre;

    vec3 col = render(scene, uv);

    col = sqrt(col);
    fragColor = vec4(col,1.0);
}