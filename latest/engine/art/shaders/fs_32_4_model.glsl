#include "model_fs.glsl"
#include "lightmap.glsl"
#include "surface.glsl"
#include "fog.glsl"

void main() {
    if (do_lightmap())
        return;
    surface_t surf = surface();

    fragcolor = surf.fragcolor;

    fragcolor.rgb = do_fog(fragcolor.rgb);
}
