glslangValidator --target-env spirv1.6 -t --client vulkan100 path-tracer.comp -o ../prebuild/shaders/path-tracer.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 post.comp -o ../prebuild/shaders/post.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 resample.comp -o ../prebuild/shaders/resample.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 reserve.comp -o ../prebuild/shaders/reserve.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 control.comp -o ../prebuild/shaders/control.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 resort.comp -o ../prebuild/shaders/resort.comp.spv

glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.frag -o ../prebuild/shaders/native-opaque.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.geom -o ../prebuild/shaders/native-opaque.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.vert -o ../prebuild/shaders/native-opaque.vert.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.frag -DTRANSLUCENT -o ../prebuild/shaders/native-translucent.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.geom -DTRANSLUCENT -o ../prebuild/shaders/native-translucent.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 native-rasterize.vert -DTRANSLUCENT -o ../prebuild/shaders/native-translucent.vert.spv

glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.frag -o ../prebuild/shaders/pre-opaque.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.geom -o ../prebuild/shaders/pre-opaque.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.vert -o ../prebuild/shaders/pre-opaque.vert.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.frag -DTRANSLUCENT -o ../prebuild/shaders/pre-translucent.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.geom -DTRANSLUCENT -o ../prebuild/shaders/pre-translucent.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 pre-rasterize.vert -DTRANSLUCENT -o ../prebuild/shaders/pre-translucent.vert.spv


echo "Let's optimize?"
pause

spirv-opt --skip-validation -O -Os ../prebuild/shaders/path-tracer.comp.spv -o ../prebuild/shaders/path-tracer.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/post.comp.spv -o ../prebuild/shaders/post.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/resample.comp.spv -o ../prebuild/shaders/resample.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/reserve.comp.spv -o ../prebuild/shaders/reserve.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/control.comp.spv -o ../prebuild/shaders/control.comp.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/resort.comp.spv -o ../prebuild/shaders/resort.comp.spv

spirv-opt --skip-validation -O -Os ../prebuild/shaders/native-opaque.frag.spv -o ../prebuild/shaders/native-opaque.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/native-opaque.geom.spv -o ../prebuild/shaders/native-opaque.geom.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/native-opaque.vert.spv -o ../prebuild/shaders/native-opaque.vert.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/native-translucent.frag.spv -o ../prebuild/shaders/native-translucent.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/native-translucent.geom.spv -o ../prebuild/shaders/native-translucent.geom.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/native-translucent.vert.spv -o ../prebuild/shaders/native-translucent.vert.spv

spirv-opt --skip-validation -O -Os ../prebuild/shaders/pre-opaque.frag.spv -o ../prebuild/shaders/pre-opaque.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/pre-opaque.geom.spv -o ../prebuild/shaders/pre-opaque.geom.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/pre-opaque.vert.spv -o ../prebuild/shaders/pre-opaque.vert.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/pre-translucent.frag.spv -o ../prebuild/shaders/pre-translucent.frag.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/pre-translucent.geom.spv -o ../prebuild/shaders/pre-translucent.geom.spv
spirv-opt --skip-validation -O -Os ../prebuild/shaders/pre-translucent.vert.spv -o ../prebuild/shaders/pre-translucent.vert.spv

pause