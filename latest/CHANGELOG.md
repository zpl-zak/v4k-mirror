*2024.9*
- Render: mesh-level sorting based on render pass
- Render: mesh-level frustum culling
- Render: Added model_bsphere()
- Render: Shadowmapping! VSM and CSM, soft shadows, directional/point/spot lights
- Render: Model auto LOD generation (WIP!)
- Render: Shader uniform caching (huge fps speedup)
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

*Past releases*
See git history stored at [misc/past_releases.txt](https://github.com/zpl-zak/v4k-mirror/blob/releases/latest/misc/past_releases.txt) for previous ancient releases.
