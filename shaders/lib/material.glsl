#ifndef MATERIAL_DEF
#define MATERIAL_DEF

// starts from 1u, count as Id-1u, alter  are Null
struct CTexture { uint32_t textureId, samplerId; };
struct TexOrDef { CTexture texture; vec4 defValue; };

//
struct MaterialInfo {
  TexOrDef texCol[MAX_MATERIAL_BIND];
};

//
struct MaterialPixelInfo {
  vec4 color[MAX_MATERIAL_BIND];
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer MaterialData {
  MaterialInfo infos[];
};

//
MaterialInfo getMaterialInfo(in GeometryInfo geometryInfo, in uint32_t materialId) {
  MaterialInfo materialInfo;
  if (geometryInfo.materialRef > 0) {
    materialInfo = MaterialData(geometryInfo.materialRef).infos[materialId];
  };
  return materialInfo;
};

//
MaterialInfo getMaterialInfo(in GeometryInfo geometryInfo) {
  return getMaterialInfo(geometryInfo, 0u);
};

//
vec4 sampleTex(CTexture tex, in vec2 texcoord, int lod) {
  //nonuniformEXT float flod = float(lod)/float(textureQueryLevels(textures[nonuniformEXT(tex.textureId)]));
  return texture(
    nonuniformEXT(sampler2D(
      textures[nonuniformEXT(tex.textureId)], 
      samplers[nonuniformEXT(tex.samplerId)]
    )), vec2(texcoord.x,texcoord.y));
};

//
vec4 sampleTex(CTexture tex, in vec2 texcoord) {
  return sampleTex(tex, texcoord, 0);
};

//
vec4 handleTexture(in TexOrDef tex, in vec2 texcoord, in uint32_t type) {
  if (tex.texture.textureId > 0u && tex.texture.textureId != -1) {
    return sampleTex(tex.texture, texcoord);
  };
  //if (type == MATERIAL_NORMAL  ) { return vec4(0.5f,0.5f,1.f,1.f); };
  //if (type == MATERIAL_ALBEDO  ) { return vec4(1.f ,1.f ,1.f,1.f); };
  //if (type == MATERIAL_EMISSIVE) { return vec4(0.f ,0.f ,0.f,0.f); };
  //if (type == MATERIAL_PBR  )    { return vec4(1.f, 0.f ,0.f,1.f); };
  return tex.defValue;
};

// without parallax mapping or normal mapping
MaterialPixelInfo handleMaterial(in MaterialInfo materialInfo, in vec2 texcoord, in mat3x3 tbn) {
  MaterialPixelInfo result;
  [[unroll]] for (uint32_t i=0;i<MAX_MATERIAL_BIND;i++) {
    result.color[i] = handleTexture(materialInfo.texCol[i], texcoord, i);
  };
  result.color[MATERIAL_NORMAL].xyz = normalize(tbn * normalize(result.color[MATERIAL_NORMAL].xyz * 2.f - 1.f));
  return result;
};

#endif
