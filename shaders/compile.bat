glslangValidator --target-env spirv1.6 -t --client vulkan100 path-tracing.comp -o ../prebuild/shaders/path-tracing.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 reprojection.comp -o ../prebuild/shaders/reprojection.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 combine.comp -o ../prebuild/shaders/combine.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 final.comp -o ../prebuild/shaders/final.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 recopy.comp -o ../prebuild/shaders/recopy.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 resort.comp -o ../prebuild/shaders/resort.comp.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 control.comp -o ../prebuild/shaders/control.comp.spv

glslangValidator --target-env spirv1.6 -t --client vulkan100 triangles.vert               -o ../prebuild/shaders/triangles.vert.spv

glslangValidator --target-env spirv1.6 -t --client vulkan100 triangles.geom               -o ../prebuild/shaders/triangles-opaque.geom.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 triangles.geom -DTRANSLUCENT -o ../prebuild/shaders/triangles-translucent.geom.spv

glslangValidator --target-env spirv1.6 -t --client vulkan100 overestimate.frag               -o ../prebuild/shaders/overestimate-opaque.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 overestimate.frag -DTRANSLUCENT -o ../prebuild/shaders/overestimate-translucent.frag.spv

glslangValidator --target-env spirv1.6 -t --client vulkan100 underestimate.frag               -o ../prebuild/shaders/underestimate-opaque.frag.spv
glslangValidator --target-env spirv1.6 -t --client vulkan100 underestimate.frag -DTRANSLUCENT -o ../prebuild/shaders/underestimate-translucent.frag.spv


echo "Let's optimize?"
pause

spirv-opt --skip-validation -O -Os ../prebuild/shaders/path-tracing.comp.spv -o ../prebuild/shaders/path-tracing.comp.spv

pause