#ifndef UTILS_DEF
#define UTILS_DEF

//
vec2 lcts(in vec3 direct) { 
  return vec2(fma(atan(direct.z,direct.x),INV_TWO_PI,0.5f), acos(direct.y)*INV_PI); 
};

//
vec3 dcts(in vec2 hr) { 
  hr = fma(hr,vec2(TWO_PI,PI), vec2(-PI,0.f));
  const float up=-cos(hr.y), over=sqrt(fma(up,-up,1.f)); 
  return vec3(cos(hr.x)*over ,up , sin(hr.x)*over); 
};

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
vec3 clampCol(in vec3 col) { return clamp(col, 0.f.xxx, 16.f.xxx); };
vec4 clampCol(in vec4 col) {
  vec4 colw = clamp(col/max(col.w, 1.f), 0.f.xxxx, vec4(16.f.xxx, 1.f));
  return colw * clamp(col.w, 1.f, 65536.f);
};

//
vec4 clampColW(in vec4 col) {
  const vec4 clamped = clampCol(col);
  return clamped/max(clamped.w, 1.f);
};

// for metallic reflection (true-multiply)
vec3 trueMultColor(in vec3 rayColor, in vec3 material) {
  const float mn = min(material.r, min(material.g, material.b));
  const float mx = max(material.r, max(material.g, material.b));
  const float chroma = (mx - mn) / absmax(mx,1e-9);
  const vec3 rayLum = luminance(max(min(sqrt(rayColor*material),rayColor),0.f.xxx)).xxx;
  return mix(rayColor, rayLum, chroma*chroma) * material;
};

// real-color technology
vec4 trueMultColor(in vec4 rayColor, in vec4 material) {
  return vec4(trueMultColor(rayColor.xyz, material.xyz), material.w * rayColor.w);
};

// for metallic reflection
vec3 metallicMult(in vec3 rayColor, in vec3 material, in float factor) {
  return mix(rayColor, trueMultColor(rayColor, material), sqrt(factor.xxx));
};

vec3 inRayNormal(in vec3 dir, in vec3 normal) {
  return normalize(faceforward(normal, dir, normal));
};

vec3 outRayNormal(in vec3 dir, in vec3 normal) {
  return normalize(faceforward(normal, -dir, normal));
};

//
vec4 divW(in vec4 coord) {
  return coord.xyzw/absmax(coord.w, 1e-9);
};

//
float sum(in vec2 f) {
  return f.x+f.y;
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

//
vec3 proj_point_in_plane(in vec3 p, in vec3 v0, in vec3 n, out float d) { return p - ((d = dot(n, p - v0)) * n); };

// m1 - view point of previous frame
// m0 - view point of current frame
// t0 - reflection point of current frame
// t1 - reflection point of previous frame
// p0 - plane point of current frame
// n0 - plane normal of current frame
// p1 - plane point of previous frame
// v0 - incident point of current frame
// v1 - incident point of previous frame
// thanks by criver#8473
vec3 find_reflection_incident_point(in vec3 m0, in vec3 t0, in vec3 p0, in vec3 n0) {
  float h1 = dot(m0-p0,n0);
  float h2 = dot(t0-p0,n0);
  vec3 c = m0 - h1 * n0;
  vec3 d = t0 - h2 * n0;
  h1 = abs(h1); h2 = abs(h2);
  vec3 v0 = mix(c,d,h1/(h1+h2));
  return v0;
};

/*vec3 find_reflection_incident_point(in vec3 p0, in vec3 p1, in vec3 v0, in vec3 n) {
  float d0 = 0; vec3 proj_p0 = proj_point_in_plane(p0, v0, n, d0);
  float d1 = 0; vec3 proj_p1 = proj_point_in_plane(p1, v0, n, d1);
  if(d1 < d0) { return (proj_p0 - proj_p1) * d1/(d0+d1) + proj_p1; }
         else { return (proj_p1 - proj_p0) * d0/(d0+d1) + proj_p0; };
};*/

// 
float qdMin(in vec2 qd) {
  return min(min(
    dot(qd, vec2(-0.5f, -0.5f)),
    dot(qd, vec2( 0.5f, -0.5f))
  ), min(
    dot(qd, vec2( -0.5f, 0.5f)),
    dot(qd, vec2(  0.5f, 0.5f))
  ));
};

// 
float qdMax(in vec2 qd) {
  return max(max(
    dot(qd, vec2(-0.5f, -0.5f)),
    dot(qd, vec2( 0.5f, -0.5f))
  ), max(
    dot(qd, vec2( -0.5f, 0.5f)),
    dot(qd, vec2(  0.5f, 0.5f))
  ));
};

// dbary => [bary00, bary10, bary01], derrived barycentric
// dpos  => [pos00, pos10, pos01], derrived position
// currenly, unused, useless
// it's reversal interpolation
mat3x4 recoverTriangle(in mat3x3 dbary, in mat3x4 dpos) {
  return dpos * inverse(transpose(dbary));
};

//
vec4 interpolate(in mat3x4 vertices, in vec3 barycentric) {
  return vertices * barycentric;
};

//
vec4 interpolate(in mat3x4 vertices, in vec2 barycentric) {
  return interpolate(vertices, vec3(1.f-barycentric.x-barycentric.y, barycentric.xy));
};

//
float edgeFunction(in vec3 a, in vec3 b, in vec3 c) { return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x); };

// required view-space as minimum
vec3 watertightTriangleIntersection(in vec4 vo, in mat3 M, in mat3x4 vt) {
  mat3x3 vt3 = M * mat3x3(vt[0].xyz-vo.xyz, vt[1].xyz-vo.xyz, vt[2].xyz-vo.xyz);
  vec3 UVW = vec3(
    vt3[2].x*vt3[1].y-vt3[2].y*vt3[1].x,
    vt3[0].x*vt3[2].y-vt3[0].y*vt3[2].x,
    vt3[1].x*vt3[0].y-vt3[1].y*vt3[0].x
  );
  float T = dot(UVW, transpose(vt3)[2].xyz);
  float det = UVW.x + UVW.y + UVW.z;
  return abs(det) > 0.f ? UVW/absmax(det, 1e-9) : vec3(0.f);
};

//
vec3 computeBary(in vec4 vo, in mat3x4 vt) {
  mat3x3 vt3 = mat3x3(vec3(vt[0].xy/absmax(vt[0].w, 1e-9), 1.f), vec3(vt[1].xy/absmax(vt[1].w, 1e-9), 1.f), vec3(vt[2].xy/absmax(vt[2].w, 1e-9), 1.f));
  float det = determinant(vt3);
  vec3 UVW = inverse(vt3)*vec3(vo.xy/absmax(vo.w, 1e-9),1.f);
  UVW /= absmax(transpose(vt)[3].xyz, 1e-9.xxx);
  UVW /= absmax(UVW.x+UVW.y+UVW.z, 1e-9);
  return abs(det) > 0.f ? UVW : vec3(0.f);
};

//
mat3x4 cvt3x4(in vec4 m[3]) { return mat3x4(m[0],m[1],m[2]); };
mat3x3 cvt3x3(in vec3 m[3]) { return mat3x3(m[0],m[1],m[2]); };
mat3x2 cvt3x2(in vec2 m[3]) { return mat3x2(m[0],m[1],m[2]); };


//
vec3 cosineWeightedPoint(in vec2 uv) {
  const float radial = sqrt(uv.x);
  const float theta = TWO_PI * uv.y;

  const float x = radial * cos(theta);
  const float y = radial * sin(theta);

  return vec3(x, y, sqrt(1 - uv.x));
};

//
vec3 cosineWeightedPoint(in vec2 uv, in mat3x3 tbn) {
  return tbn * cosineWeightedPoint(uv);
};

//
vec3 reflective(in vec2 seed, in vec3 dir, in mat3x3 tbn, in float roughness) {
  return normalize(mix(reflect(dir, tbn[2]), cosineWeightedPoint(seed, tbn), roughness));
};

const vec2 offset[25] = {
  vec2(-2,-2),
  vec2(-1,-2),
  vec2(0,-2),
  vec2(1,-2),
  vec2(2,-2),

  vec2(-2,-1), 
  vec2(-1,-1), 
  vec2(0,-1), 
  vec2(1,-1),
  vec2(2,-1),

  vec2(-2,0),
  vec2(-1,0),
  vec2(0,0),
  vec2(1,0),
  vec2(2,0),

  vec2(-2,1),
  vec2(-1,1),
  vec2(0,1),
  vec2(1,1),
  vec2(2,1),

  vec2(-2,2),
  vec2(-1,2),
  vec2(0,2),
  vec2(1,2),
  vec2(2,2)
};

const float kernel[25] = {
  1.0f/256.0f,
  1.0f/64.0f,
  3.0f/128.0f,
  1.0f/64.0f,
  1.0f/256.0f,

  1.0f/64.0f,
  1.0f/16.0f,
  3.0f/32.0f,
  1.0f/16.0f,
  1.0f/64.0f,

  3.0f/128.0f,
  3.0f/32.0f,
  9.0f/64.0f,
  3.0f/32.0f,
  3.0f/128.0f,

  1.0f/64.0f,
  1.0f/16.0f,
  3.0f/32.0f,
  1.0f/16.0f,
  1.0f/64.0f,

  1.0f/256.0f,
  1.0f/64.0f,
  3.0f/128.0f,
  1.0f/64.0f,
  1.0f/256.0f
};

//
const ivec2 CH_MAP[4] = { ivec2(1,0),ivec2(-1,0),ivec2(0,1),ivec2(0,-1) };
const ivec2 DG_MAP[4] = { ivec2(1,1),ivec2(-1,-1),ivec2(-1,1),ivec2(1,-1) };
const ivec2 FL_MAP[8] = { ivec2(1,0),ivec2(-1,0),ivec2(0,1),ivec2(0,-1),ivec2(1,1),ivec2(-1,-1),ivec2(-1,1),ivec2(1,-1) };


#endif
