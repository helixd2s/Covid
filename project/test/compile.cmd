glslangValidator --target-env spirv1.6 -t --client vulkan100 path-tracer.comp -o path-tracer.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 post.comp -o post.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 resample.comp -o resample.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 rasterize.frag -o opaque.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 rasterize.vert -o opaque.vert.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 rasterize.frag -DTRANSLUCENT -o translucent.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 rasterize.vert -DTRANSLUCENT -o translucent.vert.spv

echo "Let's optimize?"
pause

spirv-opt --skip-validation -O -Os path-tracer.comp.spv -o path-tracer.comp.spv
spirv-opt --skip-validation -O -Os post.comp.spv -o post.comp.spv
spirv-opt --skip-validation -O -Os resample.comp.spv -o resample.comp.spv
spirv-opt --skip-validation -O -Os opaque.frag.spv -o opaque.frag.spv
spirv-opt --skip-validation -O -Os opaque.vert.spv -o opaque.vert.spv
spirv-opt --skip-validation -O -Os translucent.frag.spv -o translucent.frag.spv
spirv-opt --skip-validation -O -Os translucent.vert.spv -o translucent.vert.spv

pause