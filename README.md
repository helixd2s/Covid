# ♠ Alter ♠

Some functionality still WIP, but almost ready to ***MVP***...

## Важное примечание...

  - 👑 [GitHub, документация](https://github.com/helixd2s/about)
  - 🥀 [GitHub, идеология](https://github.com/helixd2s/core) (резерв)
  - 🥀 [VK паблик](https://vk.com/helixd2s)
  - 🥀 [Наш сайт](http://about.helixd2s.su/) (там просто редирект пока что)

## Features

  - Minimal and extensible interface (Open Source, SDK)
  - Lightweight renderer as prior
  - Some automatizations (smart)
  - Ray tracing support
  - Minimal requirements
  - Vulkan API 1.3
  - Reprojections (such as reflections, diffuse, transparency)

## Planned

  - OpenGL support (interop)
  - Implementation to some games
  - Multi-Draw for instances (TLAS)
  - Pre-defined shader packages
  - Motion-vectors and animations
  - Physics interactive demo

## Future names...

  I won't go into political contexts... 

  - `ANAMED` (aka. `Alter`, will used in Minecraft)
  - `BNAMED` (aka. `Blaze`)
  - `CNAMED` (aka. `Conceit`)
  - `DNAMED` (aka. `Delta`, will used in Neverball)

## If you have issues...

### ASAN AddressSanitizer issues...

  From NVIDIA...

  ```
   Because this is an AddressSanitizer error, can you confirm the crash does not repro when run without ASAN? By default ASAN does not intercept GlobalAlloc, which leads to false positives. Because our driver uses GlobalAlloc and GlobalFree you need to set this in the environment:

  ASAN_OPTIONS=windows_hook_legacy_allocators=true

  You can read more about this runtime option here: https://docs.microsoft.com/en-us/cpp/sanitizers/asan-runtime?view=msvc-170#msvc-specific-addresssanitizer-runtime-options
  ```
