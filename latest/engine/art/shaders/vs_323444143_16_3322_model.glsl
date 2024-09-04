#include "model_vs.glsl"

// lights
#include "light.glsl"

void main() {
    vec3 objPos = get_object_pos();
    
    // Set attributes up
    v_normal_ws = normalize(vec3(att_instanced_matrix * vec4(v_normal, 0.))); // normal to world/model space
    v_normal = normalize(v_normal);
    v_position = att_position;
    v_texcoord = att_texcoord;
    v_texcoord2 = att_texcoord2;
    v_color = att_color;
    mat4 modelView = view * att_instanced_matrix;
    mat4 l_model = att_instanced_matrix;
    v_position_ws = (l_model * vec4( objPos, 1.0 )).xyz;
    vec3 binormal = cross(att_normal, att_tangent.xyz) * att_tangent.w;
    v_binormal = normalize(mat3(att_instanced_matrix) * binormal);

    // Optional: Billboarding
    {
        billboard_t bb = setup_billboard(modelView, l_model);
        modelView = bb.modelView;
        l_model = bb.l_model;
    
        v_position_ws = (l_model * vec4( objPos, 1.0 )).xyz;
        v_tangent = normalize(mat3(att_instanced_matrix) * att_tangent.xyz);
    }

    // Compute lighting (vertex-lit models)
    material_t dummy_mat;
    v_vertcolor = lighting(dummy_mat);

    // Compute final position and camera vector
    vec4 finalPos = modelView * vec4( objPos, 1.0 );
    vec3 to_camera = normalize( -finalPos.xyz );
    v_to_camera = mat3( inv_view ) * to_camera;

    // Set final position
    v_depth = -finalPos.z;
    gl_Position = P * finalPos;

    // Prepare shadow data for shadow mapping
    do_shadow(att_instanced_matrix, objPos, v_normal);
}