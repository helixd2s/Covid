glslangValidator --target-env spirv1.6 -t --client vulkan100 path-tracer.comp -o ../prebuild/path-tracer.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 post.comp -o ../prebuild/post.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 resample.comp -o ../prebuild/resample.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 control.comp -o ../prebuild/control.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 rasterize.frag -o ../prebuild/opaque.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 rasterize.vert -o ../prebuild/opaque.vert.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 rasterize.frag -DTRANSLUCENT -o ../prebuild/translucent.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 rasterize.vert -DTRANSLUCENT -o ../prebuild/translucent.vert.spv

echo "Let's optimize?"
pause

spirv-opt --skip-validation -O -Os ../prebuild/path-tracer.comp.spv -o ../prebuild/path-tracer.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/post.comp.spv -o ../prebuild/post.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/resample.comp.spv -o ../prebuild/resample.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/control.comp.spv -o ../prebuild/control.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/opaque.frag.spv -o ../prebuild/opaque.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/opaque.vert.spv -o ../prebuild/opaque.vert.spv
spirv-opt --skip-validation -O -Os ../prebuild/translucent.frag.spv -o ../prebuild/translucent.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/translucent.vert.spv -o ../prebuild/translucent.vert.spv

pause