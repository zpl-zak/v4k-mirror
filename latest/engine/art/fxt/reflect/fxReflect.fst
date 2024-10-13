// adapted from https://github.com/RoundedGlint585/ScreenSpaceReflection/blob/master/shaders/SSRFragment.glsl
uniform sampler2D u_normal_texture;
uniform sampler2D u_matprops_texture;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_inv_projection;

uniform float rayStep; /// set:0.1
uniform int iterationCount; /// set:100
uniform float distanceBias; /// set:0.03
uniform int sampleCount; /// set:0
uniform bool adaptiveStep; /// set:1
uniform bool binarySearch; /// set:1
uniform float samplingCoefficient; /// set:0.01
uniform float metallicThreshold; /// set:0.1
uniform bool debug; /// set:1

float random (vec2 uv) {
	return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123); //simple random function
}

vec3 generatePositionFromDepth(vec2 texturePos, float depth) {
	vec4 ndc = vec4((texturePos - 0.5) * 2, depth, 1.f);
	vec4 inversed = u_inv_projection * ndc;// going back from projected
	inversed /= inversed.w;
	return inversed.xyz;
}

vec2 generateProjectedPosition(vec3 pos){
	vec4 samplePosition = u_projection * vec4(pos, 1.f);
	samplePosition.xy = (samplePosition.xy / samplePosition.w) * 0.5 + 0.5;
	return samplePosition.xy;
}

vec3 SSR(vec3 position, vec3 reflection) {
    vec3 step = rayStep * reflection;
    vec3 marchingPosition = position + step;
    float delta;
    float depthFromScreen;
    vec2 screenPosition;

    int i = 0;
    for (; i < iterationCount; i++) {
        screenPosition = generateProjectedPosition(marchingPosition);

        if (screenPosition.x < 0.0 || screenPosition.x > 1.0 || screenPosition.y < 0.0 || screenPosition.y > 1.0) {
            return vec3(0.0);
        }

        depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(iChannel1, screenPosition).x).z);
        delta = abs(marchingPosition.z) - depthFromScreen;
        if (abs(delta) < distanceBias) {
            vec3 color = vec3(1.0);
            if (debug) {
                color = vec3( 0.5+ sign(delta)/2,0.3,0.5- sign(delta)/2);
            }
            return texture(iChannel0, screenPosition).rgb * color;
        }
        if (binarySearch && delta > 0.0) {
            break;
        }
        if (adaptiveStep) {
            float directionSign = sign(abs(marchingPosition.z) - depthFromScreen);
            step = step * (1.0 - rayStep * max(directionSign, 0.0));
            marchingPosition += step * (-directionSign);
        } else {
            marchingPosition += step;
        }
    }
    if (binarySearch) {
        for(; i < iterationCount; i++) {
            step *= 0.5;
            marchingPosition = marchingPosition - step * sign(delta);
            screenPosition = generateProjectedPosition(marchingPosition);
            if (screenPosition.x < 0.0 || screenPosition.x > 1.0 || screenPosition.y < 0.0 || screenPosition.y > 1.0) {
                return vec3(0.0);
            }
            depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(iChannel1, screenPosition).x).z);
            delta = abs(marchingPosition.z) - depthFromScreen;
            if (abs(delta) < distanceBias) {
                vec3 color = vec3(1.0);
                if (debug) {
                    color = vec3( 0.5+ sign(delta)/2,0.3,0.5- sign(delta)/2);
                }
                return texture(iChannel0, screenPosition).rgb * color;
            }
        }
    }
    return vec3(0.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = vec2(fragCoord.x / iChannelRes0x, fragCoord.y / iChannelRes0y);

    vec3 position = generatePositionFromDepth(uv, texture(iChannel1, uv).x);
    vec4 normal = u_view * vec4(texture(u_normal_texture, uv).xyz, 0.0);
    vec2 matprops = texture(u_matprops_texture, uv).xy;
    float metallic = matprops.x;
    if (metallic < metallicThreshold) {
        fragColor = vec4(0.0);
        return;
    }

    vec3 reflectionDirection = normalize(reflect(position, normalize(normal.xyz)));
    if (sampleCount > 0) {
        vec3 firstBasis = normalize(cross(vec3(0,0,1), reflectionDirection ));
        vec3 secondBasis = normalize(cross(reflectionDirection, firstBasis));
        vec4 resultingColor = vec4(0.0);
        for (int i = 0; i < sampleCount; i++) {
            vec2 coeffs = vec2(random(uv + vec2(0,i)) + random(uv + vec2(i,0))) * samplingCoefficient;
            vec3 reflectionDirectionRandomized = reflectionDirection + firstBasis * coeffs.x + secondBasis * coeffs.y;
            vec3 tempColor = SSR(position, normalize(reflectionDirectionRandomized));
            if (tempColor != vec3(0.0)) {
                resultingColor += vec4(tempColor, 1.0);
            }
        }
        if (resultingColor.w == 0) {
            fragColor = vec4(0.0);
        } else {
            resultingColor /= resultingColor.w;
            fragColor = vec4(resultingColor.xyz, 1.0);
        }
    } else {
        fragColor = vec4(SSR(position, reflectionDirection), 1.0);

        if (fragColor.xyz == vec3(0.0)) {
            fragColor.a = 0.0;
        }
    }
}