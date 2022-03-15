# 💔`ZERO`💔

Some functionality still WIP, but almost ready to ***MVP***...

## Features

- Minimal and extensible interface (Open Source, SDK)
- Lightweight renderer as prior
- Some automatizations (smart)
- Ray tracing support
- Minimal requirements
- Vulkan API 1.3

## Planned

- OpenGL support (interop)
- Implementation to some games
- Multi-Draw for instances (TLAS)
- Pre-defined shader packages

## Code names and history

- [x] ♠`LXVC` (**L**ightweight, e**X**tensible, **V**ulkan, **C**omplex or **C**ompatible)
- [x] 💔`ZERO` (`ZNAMED`)
- [ ] 🇷🇺 `ALPHA` or 🇺🇦 `AZOV` (`ANAMED`)
- [ ] 🔥`BLAZE(R?)` (`BNAMED`)
- [ ] `CNAMED`
- [ ] `DNAMED`
- [ ] etc. by alpha-beta...

## If you have issues...

### ASAN AddressSanitizer issues...

From NVIDIA...

```
 Because this is an AddressSanitizer error, can you confirm the crash does not repro when run without ASAN? By default ASAN does not intercept GlobalAlloc, which leads to false positives. Because our driver uses GlobalAlloc and GlobalFree you need to set this in the environment:

ASAN_OPTIONS=windows_hook_legacy_allocators=true

You can read more about this runtime option here: https://docs.microsoft.com/en-us/cpp/sanitizers/asan-runtime?view=msvc-170#msvc-specific-addresssanitizer-runtime-options
```
