# 🌋 Alter 🌋

Some functionality still WIP, but almost ready to ***MVP***…

## Важное примечание…

  - 👑 [GitHub, документация](https://github.com/helixd2s/about)
  - 🥀 [GitHub, идеология](https://github.com/helixd2s/core) (резерв)
  - 🥀 [VK паблик](https://vk.com/helixd2s)
  - 🥀 [Наш сайт](http://about.helixd2s.su/) (там просто редирект пока что)

## Renderer features

  - [x] ♟️ Almost full Vulkan API 1.3
  - [x] ♟️ Minimal and extensible interface (Open Source, SDK)
  - [x] ♟️ Native ray tracing support (HW/RT cores, ray-query)
  - [x] ♟️ Some automatizations (incl. barriers, etc.)
  - [x] ♟️ Multi-draw of geometries or meshes
  - [x] ♟️ Graphics and compute shaders support
  - [x] ♟️ Async-model support, with sort of GC (alike JS)
  - [x] ♟️ Input lockless and independent rendering
  - [x] ♟️ Partial basic multi-threading support
  - [x] ♟️ Using same pipelines for rendering
  - [x] ♟️ Almost fully bindless model
  - [x] ♟️ Basic two-level rendering model
  - [ ] 🧩 More extensions support
  - [ ] 🧩️ OpenGL support (interop)
  - [ ] 🧩 Full subgroups support (incl. variable)
  - [ ] 🧩 Interface wrapper and SDK (for C/C++)
  - [ ] 🧩 Better instancing support (reusing, indexing)
  - [ ] 🧩️ Multi-draw with instances (TLAS)
  - [ ] 🧩 Secondary or alternative pipelines
  - [ ] 🧩 Ray-tracing pipelines (currently ray-query)
  - [ ] 👑 Full and native ReShade support
  - [ ] 👑 Interop with CUDA and OpenCL (**not** AMD)
  - [ ] 👑 Interop with DirectX 12
  - [ ] 👑 DirectX 12 underlayer of Vulkan
  - [ ] 👑️ Implementation for some games

## Built-in shaders SDK features

  - [x] ♟️ Test shaders (currently, not a library)
  - [x] ♟️ Reprojection of reflections and transparency
  - [x] ♟️ Dynamic vertex pulling and rendering
  - [x] ♟️ Checkerboard optimization (for diffuse/reflection)
  - [x] ♟️ Temporal screen-space reprojection (accumulation)
  - [ ] 🧩 Software rasterization misc (barycentrics, vector-sampling)
  - [ ] 🧩 Rewrite to HLSL (needs features)
  - [ ] 🧩 Distances, MIP and LoD support
  - [ ] 🧩 Full vulkan memory model support
  - [ ] 🧩️ Pre-defined shader packages
  - [ ] 🧩 Short-range SSRT support
  - [ ] 🧩 Advanced shadow maps (with simpler RT)

## Integrations

  - [x] ☄️ KTX-2 format support
  - [x] ☄️ Basic GLTF support (single model, PBR)
  - [ ] 👑 Radix sort for all GPU (GLSL and HLSL)

## Testing and demo initialive 

  - [x] ☄️ Env. map testing
  - [x] ☄️ Argument passing for app
  - [x] ☄️ Basic controller (camera moving)
  - [ ] ☄️ Basic OBJ support
  - [ ] ☄️ Physics demo (interactive, dynamic)
  - [ ] ☄️ Advanced GLTF support (more than one model, more features)
  - [ ] ☄️ GLTF animations support
  - [ ] ☄️ GUI controllers and better interactivity
  - [ ] ☄️ Motion-vectors and animations
  
## Minecraft and Java 16 spec-operation (project `Alpha`)…

  - [ ] ☕ Latest LWJGL loading, interop and support
  - [ ] ☕ JNI, JavaCPP and Java support (lossless)
  - [ ] ☕ Correct Kotlin support (subset of Java)
  - [ ] ☕ OpenGL interop and compatibility
  - [ ] ☕ Interop with VMA (almost done)
  - [ ] ☕ Minecraft mod itself (Forge)
  - [ ] ☕ Support for Fabric (mod, library, api)
  - [ ] ☕ Support for 1.18.2 and/or beyond
  - [ ] 👑 Support for Rendering API and Blaze3D
  - [ ] 👑 Support for OptiFabric (Fabric)
  - [ ] 👑 Support for Immersive Portals Mod (Fabric)
  - [ ] 👑 Support for Forge (mod, library)
  - [ ] 👑 Support for Optifine (Forge)

## If you have issues…

### ASAN AddressSanitizer issues…

  From NVIDIA…

  ```
   Because this is an AddressSanitizer error, can you confirm the crash does not repro when run without ASAN? By default ASAN does not intercept GlobalAlloc, which leads to false positives. Because our driver uses GlobalAlloc and GlobalFree you need to set this in the environment:

  ASAN_OPTIONS=windows_hook_legacy_allocators=true

  You can read more about this runtime option here: https://docs.microsoft.com/en-us/cpp/sanitizers/asan-runtime?view=msvc-170#msvc-specific-addresssanitizer-runtime-options
  ```
