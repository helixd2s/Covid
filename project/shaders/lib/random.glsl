#ifndef RANDOM_DEF
#define RANDOM_DEF

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu;       // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u;       // 1.0 in IEEE binary32

    m &= ieeeMantissa;                           // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                                // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );             // Range [1:2]
    return clamp(fract(f - 1.0), 0.f, 0.99999f); // Range [0:1]
}

//
uint rndCount = 0;

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(uvec4(floatBitsToUint(x), frameCounter + (rndCount++), clockRealtime2x32EXT()))); };
float random( vec2  v ) { return floatConstruct(hash(uvec4(floatBitsToUint(v.x), floatBitsToUint(v.y) + frameCounter + (rndCount++), clockRealtime2x32EXT()))); };
float random( vec3  v ) { return floatConstruct(hash(uvec4(floatBitsToUint(v.x), floatBitsToUint(v.y) + frameCounter + (rndCount++), uvec2(floatBitsToUint(v.z), 0u) + clockRealtime2x32EXT()))); };
float random( vec4  v ) { return floatConstruct(hash(uvec4(floatBitsToUint(v.x), floatBitsToUint(v.y) + frameCounter + (rndCount++), uvec2(floatBitsToUint(v.z), floatBitsToUint(v.w)) + clockRealtime2x32EXT()))); };

#endif
