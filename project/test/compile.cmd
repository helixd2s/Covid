glslangValidator --target-env spirv1.6 --client vulkan100 test.comp -o test.comp.spv
glslangValidator --target-env spirv1.6 --client vulkan100 rasterize.frag -o opaque.frag.spv
glslangValidator --target-env spirv1.6 --client vulkan100 rasterize.vert -o opaque.vert.spv
glslangValidator --target-env spirv1.6 --client vulkan100 rasterize.frag -DTRANSLUCENT -o translucent.frag.spv
glslangValidator --target-env spirv1.6 --client vulkan100 rasterize.vert -DTRANSLUCENT -o translucent.vert.spv
pause