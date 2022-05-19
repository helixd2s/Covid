#ifndef UTILS_DEF
#define UTILS_DEF

//
vec2 lcts(in vec3 direct) { return vec2(fma(atan(direct.z,direct.x),INV_TWO_PI,0.5f), acos(direct.y)*INV_PI); };
vec3 dcts(in vec2 hr) { hr = fma(hr,vec2(TWO_PI,PI),vec2(-PI,0.f)); const float up=-cos(hr.y),over=sqrt(fma(up,-up,1.f)); return vec3(cos(hr.x)*over,up,sin(hr.x)*over); };

//
vec3 fromLinear(in vec3 linearRGB) { return mix(vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055), linearRGB * vec3(12.92), lessThan(linearRGB, vec3(0.0031308))); }
vec4 fromLinear(in vec4 linearRGB) { return vec4(fromLinear(linearRGB.xyz), linearRGB.w); }
vec3 toLinear(in vec3 sRGB) { return mix(pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4)), sRGB/vec3(12.92), lessThan(sRGB, vec3(0.04045))); }
vec4 toLinear(in vec4 sRGB) { return vec4(toLinear(sRGB.xyz), sRGB.w); }

//
vec3 gamma(in vec3 c) { return pow(c.rgb, 1.f.xxx/HDR_GAMMA); };
vec3 ungamma(in vec3 c) { return pow(c.rgb, HDR_GAMMA.xxx); };
vec4 gamma(in vec4 c) { return vec4(gamma(c.rgb), c.a); };
vec4 ungamma(in vec4 c) { return vec4(ungamma(c.rgb), c.a); };

//
vec3 gamma3(in vec3 c) { return gamma(c.rgb) * 3.f; };
vec3 ungamma3(in vec3 c) { return ungamma(c.rgb) * 3.f; };
vec4 gamma3(in vec4 c) { return vec4(gamma3(c.rgb), c.a); };
vec4 ungamma3(in vec4 c) { return vec4(ungamma3(c.rgb), c.a); };

//
uint sgn(in uint val) { return uint(0 < val) - uint(val < 0); }
uint tiled(in uint sz, in uint gmaxtile) {
  return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
};

//
mat3x4 inverse(in mat3x4 inmat) {
  //const mat4x4 temp = transpose(inverse(transpose(mat4x4(inmat[0],inmat[1],inmat[2],vec4(0.f.xxx,1.f)))));
  const mat4x4 temp = inverse(mat4x4(inmat[0],inmat[1],inmat[2],vec4(0.f.xxx,1.f)));
  return mat3x4(temp[0],temp[1],temp[2]);
};

//
mat3x3 toNormalMat(in mat3x4 inmat) {
  //return transpose(inverse(transpose(mat3x3(inmat))));
  return inverse(transpose(mat3x3(inmat)));
};

//
float luminance(in vec3 color) {
  return dot(vec3(color), vec3(0.3f, 0.59f, 0.11f));
};

// for metallic reflection (true-multiply)
vec3 trueMultColor(in vec3 rayColor, in vec3 material) {
  const float rfactor = clamp(luminance(max(rayColor,0.f.xxx)), 0.f, 16.f);
  const float mfactor = clamp(luminance(max(material,0.f.xxx)), 0.f, 16.f);
  return clamp(material,0.f.xxx,16.f.xxx) * clamp(rayColor,0.f.xxx,16.f.xxx);
};

vec4 trueMultColor(in vec4 rayColor, in vec4 material) {
  return vec4(trueMultColor(rayColor.xyz, material.xyz), material.w * rayColor.w);
};

// for metallic reflection
vec3 metallicMult(in vec3 rayColor, in vec3 material, in float factor) {
  const float rfactor = clamp(luminance(max(rayColor,0.f.xxx)), 0.f, 16.f);
  const float mfactor = clamp(luminance(max(material,0.f.xxx)), 0.f, 16.f);
  return mix(clamp(rayColor,0.f.xxx,16.f.xxx), mix(rayColor * material, rfactor.xxx * material, max(material.r, max(material.g, material.b)) - min(material.r, min(material.g, material.b))), factor.xxx);
};

vec3 inRayNormal(in vec3 dir, in vec3 normal) {
  return normalize(faceforward(normal, dir, normal));
};

vec3 outRayNormal(in vec3 dir, in vec3 normal) {
  return normalize(faceforward(normal, dir, normal));
};

vec4 clampCol(in vec4 col) {
  return clamp(max(col,0.f.xxxx)/max(col.w, 1.f), vec4(0.f.xxx, 1.f), vec4(16.f.xxx, 1.f));
};

float absmax(in float val, in float mn) {
  float sig = sign(val); return mix(mn, sig * max(abs(val), abs(mn)), abs(sig) > 0.f);
};

vec2 absmax(in vec2 val, in vec2 mn) {
  vec2 sig = sign(val); return mix(mn, sig * max(abs(val), abs(mn)), greaterThan(abs(sig), 0.f.xx));
};

vec3 absmax(in vec3 val, in vec3 mn) {
  vec3 sig = sign(val); return mix(mn, sig * max(abs(val), abs(mn)), greaterThan(abs(sig), 0.f.xxx));
};

vec4 absmax(in vec4 val, in vec4 mn) {
  vec4 sig = sign(val); return mix(mn, sig * max(abs(val), abs(mn)), greaterThan(abs(sig), 0.f.xxxx));
};

//
vec4 divW(in vec4 coord) {
  return coord.xyzw/absmax(coord.w, 1e-9);
};

//
void genTB(in vec3 N, out vec3 T, out vec3 B) {
  const float s = N.z < 0.0 ? -1.0 : 1.0;
  const float a = -1.0 / (s + N.z);
  const float b = N.x * N.y * a;
  T = vec3(1.0 + s * N.x * N.x * a, s * b, -s * N.x);
  B = vec3(b, s + N.y * N.y * a, -N.y);
};

//
vec3 coneSample(in vec3 N, in float cosTmax, in vec2 r) {
  vec3 T, B; genTB(N, T, B);
  r.x *= 2.0 * PI;
  r.y = 1.0 - r.y * (1.0 - cosTmax);
  const float s = sqrt(1.0 - r.y * r.y);
  return T * (cos(r.x) * s) + B * (sin(r.x) * s) + N * r.y;
};

//
bool intersect(in vec4 sphere, in vec3 O, in vec3 D, inout float tmax) {
  const vec3 L = sphere.xyz - O;
  const float tc = dot(D, L);
  const float t = tc - sqrt(sphere.w * sphere.w + tc * tc - dot(L, L));
  if (t > 0.0 && t < tmax) {
    tmax = t; return true;
  }
  return false;
};

#endif
