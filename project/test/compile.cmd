glslangValidator --target-env spirv1.6 --client vulkan100 test.comp -o test.comp.spv
glslangValidator --target-env spirv1.6 --client vulkan100 post.comp -o post.comp.spv
glslangValidator --target-env spirv1.6 --client vulkan100 resample.comp -o resample.comp.spv
glslangValidator --target-env spirv1.6 --client vulkan100 rasterize.frag -o opaque.frag.spv
glslangValidator --target-env spirv1.6 --client vulkan100 rasterize.vert -o opaque.vert.spv
glslangValidator --target-env spirv1.6 --client vulkan100 rasterize.frag -DTRANSLUCENT -o translucent.frag.spv
glslangValidator --target-env spirv1.6 --client vulkan100 rasterize.vert -DTRANSLUCENT -o translucent.vert.spv

echo "Let's optimize?"
pause

spirv-opt --skip-validation -O test.comp.spv -o test.comp.spv
spirv-opt --skip-validation -O post.comp.spv -o post.comp.spv
spirv-opt --skip-validation -O resample.comp.spv -o resample.comp.spv
spirv-opt --skip-validation -O opaque.frag.spv -o opaque.frag.spv
spirv-opt --skip-validation -O opaque.vert.spv -o opaque.vert.spv
spirv-opt --skip-validation -O translucent.frag.spv -o translucent.frag.spv
spirv-opt --skip-validation -O translucent.vert.spv -o translucent.vert.spv

pause