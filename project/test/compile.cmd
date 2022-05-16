glslangValidator --target-env spirv1.6 -t --client vulkan100 path-tracer.comp -o ../prebuild/path-tracer.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 post.comp -o ../prebuild/post.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 resample.comp -o ../prebuild/resample.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 control.comp -o ../prebuild/control.comp.spv

glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.frag -o ../prebuild/native-opaque.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.geom -o ../prebuild/native-opaque.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.vert -o ../prebuild/native-opaque.vert.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.frag -DTRANSLUCENT -o ../prebuild/native-translucent.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.geom -DTRANSLUCENT -o ../prebuild/native-translucent.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.vert -DTRANSLUCENT -o ../prebuild/native-translucent.vert.spv

glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.frag -o ../prebuild/pre-opaque.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.geom -o ../prebuild/pre-opaque.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.vert -o ../prebuild/pre-opaque.vert.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.frag -DTRANSLUCENT -o ../prebuild/pre-translucent.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.geom -DTRANSLUCENT -o ../prebuild/pre-translucent.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.vert -DTRANSLUCENT -o ../prebuild/pre-translucent.vert.spv


echo "Let's optimize?"
pause

spirv-opt --skip-validation -O -Os ../prebuild/path-tracer.comp.spv -o ../prebuild/path-tracer.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/post.comp.spv -o ../prebuild/post.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/resample.comp.spv -o ../prebuild/resample.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/control.comp.spv -o ../prebuild/control.comp.spv

spirv-opt --skip-validation -O -Os ../prebuild/native-opaque.frag.spv -o ../prebuild/native-opaque.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/native-opaque.geom.spv -o ../prebuild/native-opaque.geom.spv
spirv-opt --skip-validation -O -Os ../prebuild/native-opaque.vert.spv -o ../prebuild/native-opaque.vert.spv
spirv-opt --skip-validation -O -Os ../prebuild/native-translucent.frag.spv -o ../prebuild/native-translucent.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/native-translucent.geom.spv -o ../prebuild/native-translucent.geom.spv
spirv-opt --skip-validation -O -Os ../prebuild/native-translucent.vert.spv -o ../prebuild/native-translucent.vert.spv

spirv-opt --skip-validation -O -Os ../prebuild/pre-opaque.frag.spv -o ../prebuild/pre-opaque.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/pre-opaque.geom.spv -o ../prebuild/pre-opaque.geom.spv
spirv-opt --skip-validation -O -Os ../prebuild/pre-opaque.vert.spv -o ../prebuild/pre-opaque.vert.spv
spirv-opt --skip-validation -O -Os ../prebuild/pre-translucent.frag.spv -o ../prebuild/pre-translucent.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/pre-translucent.geom.spv -o ../prebuild/pre-translucent.geom.spv
spirv-opt --skip-validation -O -Os ../prebuild/pre-translucent.vert.spv -o ../prebuild/pre-translucent.vert.spv

pause