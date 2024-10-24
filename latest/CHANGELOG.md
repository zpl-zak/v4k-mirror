*2024.9*
- Render: mesh-level sorting based on render pass
- Render: mesh-level frustum culling
- Render: Added model_bsphere()
- Render: Shadowmapping! VSM and CSM, soft shadows, directional/point/spot lights
- Render: Model auto LOD generation (WIP!)
- Render: Shader uniform caching (huge fps speedup)
- Render: Reworked model instancing to avoid glBufferData() overhead for single-instance models
- Render: added `model_get_bone_position()`
- Render: Implemented VertexLit shading (for the cool PSX looks!)
- Render: Improved vertex color support
- Render: Fixed wrong axis order used during model import (BREAKING!)
- Render: Revamped the model API (BREAKING!)
- Render: Added fog support (WIP!)
- Render: Revamped the shader system to be more flexible and modular
- Render: Introduction of the shader library (see `engine/art/shaderlib/`)
- Render: Improved transparency detection and support
- Render: Reworked skybox and cubemap modules
- Render: Added environment probes baking
- Render: Added light probes and spherical harmonics weighting
- Render: Dropped support for the lightmapper API
- Scene: Integrated shadowmap support
- Scene: Implemented renderbuckets (WIP!)
- Scene/Render: Moved light to render module
- Render: removed access to the model shader program
- Render: introduced `model_uniform_t` a new approach to shader uniform management
- Render: switched to PBR workflow as the default
- Render: model shader now uses fixed uniform sampler slots. For user bound slots, use: `model_texture_unit(model_t-)` to generate a new ID on a rolling counter
- Render: model uniforms are now cached
- Render: shadow filter size raised to 6, maximum shadow lights raised to 4, max total lights raised to 96
- Render: UBO creation/binding api
- Render: model shader now stores lights in an UBO resource
- Render: improved fallback to non-PBR textures in PBR workflow
- Render: model shader BRDF can now sample a cubemap for irradiance in IBL path (WIP!)
- Render: PBR workflow is now the default for loaded models
- Render: added model_sync() to update model data from CPU to GPU (WIP!)
- Camera: added camera_freefly() and replaced all freefly camera code in demos
- Render: cascaded shadow mapping has an improved bias
- Platform: added alert_caption() and tweaked PANIC() to copy error to clipboard
- Render: implemented parallax occlusion mapping and self-shadowing support
- Render: introduced HAS_TEXTURE_QUERY_LOD to check for support at runtime
- Window: added window_set_resolution(width, height)
- Render: added renderstate_checksum() to check for render state changes
- Render: dynamic batching support for models

*Past releases*
See git history stored at [misc/past_releases.txt](https://github.com/zpl-zak/v4k-mirror/blob/releases/latest/misc/past_releases.txt) for previous ancient releases.
