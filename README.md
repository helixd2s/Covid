# 🌋 Alter 🌋

Some functionality still WIP, but almost ready to ***MVP***…

## Важное примечание…

  - 👑 [GitHub, документация](https://github.com/helixd2s/about)
  - 🥀 [GitHub, идеология](https://github.com/helixd2s/core) (резерв)
  - 🥀 [VK паблик](https://vk.com/helixd2s)
  - 🥀 [Наш сайт](http://about.helixd2s.su/) (там просто редирект пока что)

## Features

  - [x] ♟️ Minimal and extensible interface (Open Source, SDK)
  - [x] ♟️ Lightweight renderer as prior
  - [x] ♟️ Some automatizations (smart)
  - [x] ♟️ Ray tracing support
  - [x] ♟️ Minimal requirements
  - [x] ♟️ Vulkan API 1.3
  - [x] ♟️ Reprojections (such as reflections, diffuse, transparency)
  - [x] ♟️ Test shaders (currently, not a library)
  - [x] ♟️ Checkerboard optimization

## Planned features 

  - [ ] 🧩 Distances, MIP and LoD support 
  - [ ] 🧩 Short-range SSRT support
  - [ ] 🧩 More extensions support
  - [ ] 🧩️ OpenGL support (interop)
  - [ ] 🧩️ Multi-Draw for instances (TLAS)
  - [ ] 🧩️ Pre-defined shader packages
  - [ ] 🧩 Motion-vectors and animations
  - [ ] 🧩 Full and native ReShade support
  - [ ] 👑️ Implementation to some games

## Testing and demo initialive 

  - [x] ☄️ Argument passing for app
  - [x] ☄️ Basic controller (camera moving)
  - [x] ☄️ Basic GLTF support (single model, PBR)
  - [ ] ☄️ Basic OBJ support
  - [ ] ☄️ Physics demo (interactive, dynamic)
  - [ ] ☄️ Advanced GLTF support (more than one model, more features)
  - [ ] ☄️ GLTF animations support
  - [ ] ☄️ GUI controllers and better interactivity

## Minecraft and Java 16 spec-operation (project `Alpha`)…

  - [ ] ☕ Latest LWJGL loading, interop and support
  - [ ] ☕ JNI, JavaCPP and Java support (lossless)
  - [ ] ☕ Correct Kotlin support (subset of Java)
  - [ ] ☕ OpenGL interop and compatibility
  - [ ] ☕ Interop with VMA (almost done)
  - [ ] ☕ Minecraft mod itself (Forge)
  - [ ] ☕ Support for Fabric (mod, library, api)
  - [ ] ☕ Support for 1.18.2 and/or beyond
  - [ ] 👑 Support for Rendering API and Blaze 3D
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
