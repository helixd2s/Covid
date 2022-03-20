glslangValidator --target-env spirv1.6 --client vulkan100 test.comp -o test.comp.spv
glslangValidator --target-env spirv1.6 --client vulkan100 opaque.frag -o opaque.frag.spv
glslangValidator --target-env spirv1.6 --client vulkan100 opaque.vert -o opaque.vert.spv
pause