 glslc --target-env=vulkan1.3 path-tracing.comp -o ../prebuild/shaders/path-tracing.comp.spv
 glslc --target-env=vulkan1.3 reprojection.comp -o ../prebuild/shaders/reprojection.comp.spv
 glslc --target-env=vulkan1.3 combine.comp -o ../prebuild/shaders/combine.comp.spv
 glslc --target-env=vulkan1.3 final.comp -o ../prebuild/shaders/final.comp.spv
 glslc --target-env=vulkan1.3 recopy.comp -o ../prebuild/shaders/recopy.comp.spv
 glslc --target-env=vulkan1.3 resort.comp -o ../prebuild/shaders/resort.comp.spv
 glslc --target-env=vulkan1.3 control.comp -o ../prebuild/shaders/control.comp.spv

 glslc --target-env=vulkan1.3 triangles.vert               -o ../prebuild/shaders/triangles.vert.spv

 glslc --target-env=vulkan1.3 triangles.geom               -o ../prebuild/shaders/triangles-opaque.geom.spv
 glslc --target-env=vulkan1.3 triangles.geom -DTRANSLUCENT -o ../prebuild/shaders/triangles-translucent.geom.spv

 glslc --target-env=vulkan1.3 overestimate.frag               -o ../prebuild/shaders/overestimate-opaque.frag.spv
 glslc --target-env=vulkan1.3 overestimate.frag -DTRANSLUCENT -o ../prebuild/shaders/overestimate-translucent.frag.spv

 glslc --target-env=vulkan1.3 underestimate.frag               -o ../prebuild/shaders/underestimate-opaque.frag.spv
 glslc --target-env=vulkan1.3 underestimate.frag -DTRANSLUCENT -o ../prebuild/shaders/underestimate-translucent.frag.spv


echo "Let's optimize?"
pause

spirv-opt --skip-validation -O -Os ../prebuild/shaders/path-tracing.comp.spv -o ../prebuild/shaders/path-tracing.comp.spv

pause