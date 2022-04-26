/*
   sMath, v0.1 (WIP)
   * no dependencies
   * simple
   Do this:
	  #define SEMPER_MATH_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define SEMPER_MATH_IMPLEMENTATION
   #include "sMath.h"
*/

// TODO: Optimize and take advantage of SSE/SIMD where available

#ifndef SEMPER_MATH_H
#define SEMPER_MATH_H

#ifndef S_MATH_ASSERT
#include <assert.h>
#define S_MATH_ASSERT(x) assert(x)
#endif

#include <cmath>

static constexpr float S_E        = 2.71828182f; // e
static constexpr float S_LOG2E    = 1.44269504f; // log2(e)
static constexpr float S_LOG10E   = 0.43429448f; // log10(e)
static constexpr float S_LN2      = 0.69314718f; // ln(2)
static constexpr float S_LN10     = 2.30258509f; // ln(10)
static constexpr float S_PI       = 3.14159265f; // pi
static constexpr float S_PI_2     = 1.57079632f; // pi/2
static constexpr float S_PI_3     = 1.04719755f; // pi/3
static constexpr float S_PI_4     = 0.78539816f; // pi/4
static constexpr float S_1_PI     = 0.31830988f; // 1/pi
static constexpr float S_2_PI     = 0.63661977f; // 2/pi
static constexpr float S_2_SQRTPI = 1.12837916f; // 2/sqrt(pi)
static constexpr float S_SQRT2    = 1.41421356f; // sqrt(2)
static constexpr float S_SQRT1_2  = 0.70710678f; // 1/sqrt(2)

static constexpr double S_PI_D = 3.1415926535897932; // pi

union sVec2
{
	struct { float x, y; };
	struct { float r, g; };
	struct { float u, v; };
	float d[2];
	inline sVec2() = default;
	inline sVec2(float x, float y) : x(x), y(y){}
	inline float& operator[](int index)      { return d[index];}
	inline float operator[](int index) const { return d[index];}
};

union sVec3
{
	struct { float x, y, z; };
	struct { float r, g, b; };
	struct { float u, v, __; };
	struct { sVec2 xy; float ignore0_; };
	struct { sVec2 rg; float ignore1_; };
	struct { sVec2 uv; float ignore2_; };
	struct { float ignore3_; sVec2 yz; };
	struct { float ignore4_; sVec2 gb; };
	struct { float ignore5_; sVec2 v__; };
	float d[3];
	inline sVec3() = default;
	inline sVec3(float x, float y, float z) : x(x), y(y), z(z){}
	inline float& operator[](int index)      { return d[index];}
	inline float operator[](int index) const { return d[index];}
};

union sVec4
{
	struct
	{
		union
		{
			sVec3 xyz;
			struct{ float x, y, z;};
		};

		float w;
	};
	struct
	{
		union
		{
			sVec3 rgb;
			struct{ float r, g, b;};
		};
		float a;
	};
	struct
	{
		sVec2 xy;
		float ignored0_, ignored1_;
	};
	struct
	{
		float ignored2_;
		sVec2 yz;
		float ignored3_;
	};
	struct
	{
		float ignored4_, ignored5_;
		sVec2 zw;
	};

	float d[4];
	inline sVec4() = default;
	inline sVec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w){}
	inline float& operator[](int index)      { return d[index];}
	inline float operator[](int index) const { return d[index];}
};

struct sMat4  // column major
{
	sVec4 cols[4];

	sMat4() = default;

	inline sMat4(float value)
	{
		cols[0][0] = value;
		cols[0][1] = cols[0][2] = cols[0][3] = 0.0f;
		cols[1][1] = value;
		cols[1][0] = cols[1][2] = cols[1][3] = 0.0f;
		cols[2][2] = value;
		cols[2][0] = cols[2][1] = cols[2][3] = 0.0f;
		cols[3][3] = value;
		cols[3][0] = cols[3][1] = cols[3][2] = 0.0f;
	}

	inline sMat4(float x00, float x11, float x22, float x33)
	{
		cols[0][0] = x00;
		cols[0][1] = cols[0][2] = cols[0][3] = 0.0f;
		cols[1][1] = x11;
		cols[1][0] = cols[1][2] = cols[1][3] = 0.0f;
		cols[2][2] = x22;
		cols[2][0] = cols[2][1] = cols[2][3] = 0.0f;
		cols[3][3] = x33;
		cols[3][0] = cols[3][1] = cols[3][2] = 0.0f;
	}

	inline sMat4(sVec4 c0, sVec4 c1, sVec4 c2, sVec4 c3)
	{
		cols[0] = c0;
		cols[1] = c1;
		cols[2] = c2;
		cols[3] = c3;
	}

	inline sVec4& operator[](int index)      { return cols[index];}
	inline sVec4 operator[](int index) const { return cols[index];}
	inline float& at(int index)              { return cols[index / 4][index % 4];}
	inline float at(int index) const         { return cols[index / 4][index % 4];}
};

// operator overloads
inline sVec2 operator+(sVec2 left, sVec2 right) { return {left.x+right.x, left.y+right.y};}
inline sVec3 operator+(sVec3 left, sVec3 right) { return {left.x+right.x, left.y+right.y, left.z+right.z};}
inline sVec4 operator+(sVec4 left, sVec4 right) { return {left.x+right.x, left.y+right.y, left.z+right.z, left.w+right.w};}

inline sVec2 operator-(sVec2 left, sVec2 right) { return {left.x-right.x, left.y-right.y};}
inline sVec3 operator-(sVec3 left, sVec3 right) { return {left.x-right.x, left.y-right.y, left.z-right.z};}
inline sVec4 operator-(sVec4 left, sVec4 right) { return {left.x-right.x, left.y-right.y, left.z-right.z, left.w-right.w};}

inline sVec2 operator*(sVec2 left, sVec2 right){ return {left.x*right.x, left.y*right.y};}
inline sVec3 operator*(sVec3 left, sVec3 right){ return {left.x*right.x, left.y*right.y, left.z*right.z};}
inline sVec4 operator*(sVec4 left, sVec4 right){ return {left.x*right.x, left.y*right.y, left.z*right.z, left.w*right.w};}
sMat4 operator*(sMat4 left, sMat4 right);

inline sMat4 operator*(sMat4 left, float right) { sMat4 result = left; for (unsigned i = 0; i < 4; i++) for (unsigned j = 0; j < 4; j++) result[i][j] *= right; return result;}
inline sMat4 operator*(float left, sMat4 right) { sMat4 result = right; for (unsigned i = 0; i < 4; i++) for (unsigned j = 0; j < 4; j++) result[i][j] *= left; return result;}
inline sVec2 operator*(sVec2 left, float right) { return {left.x*right, left.y*right};}
inline sVec3 operator*(sVec3 left, float right) { return {left.x*right, left.y*right, left.z*right};}
inline sVec4 operator*(sVec4 left, float right) { return {left.x*right, left.y*right, left.z*right, left.w*right};}
inline sVec2 operator*(float left, sVec2 right) { return {left*right.x, left*right.y};}
inline sVec3 operator*(float left, sVec3 right) { return {left*right.x, left*right.y, left*right.z};}
inline sVec4 operator*(float left, sVec4 right) { return {left*right.x, left*right.y, left*right.z, left*right.w};}
sVec3 operator*(sMat4 left, sVec3 right);
sVec4 operator*(sMat4 left, sVec4 right);

inline sVec2 operator/(sVec2 left, float right) { return {left.x/right, left.y/right};}
inline sVec3 operator/(sVec3 left, float right) { return {left.x/right, left.y/right, left.z/right};}
inline sVec4 operator/(sVec4 left, float right) { return {left.x/right, left.y/right, left.z/right, left.w/right};}
inline sVec2 operator/(float left, sVec2 right) { return {left/right.x, left/right.y};}
inline sVec3 operator/(float left, sVec3 right) { return {left/right.x, left/right.y, left/right.z};}
inline sVec4 operator/(float left, sVec4 right) { return {left/right.x, left/right.y, left/right.z, left/right.w};}

namespace Semper
{

	// misc. utilities
	inline float to_radians(float degrees)      { return degrees * 0.0174532925f;}
	inline float to_degrees(float radians)      { return radians * 57.29577951f;}
	inline float get_max   (float v1, float v2) { return v1 > v2 ? v1 : v2;}
	inline float get_min   (float v1, float v2) { return v1 < v2 ? v1 : v2;}
	inline float get_floor (float value)        { return floorf(value);}
	inline float log2      (float value)        { return log2f(value);}
	inline float square    (float value)        { return value*value;}
	inline float cube      (float value)        { return value*value*value;}
	inline float clamp     (float minV, float value, float maxV) { if (value < minV) return minV; else if (value > maxV) return maxV; return value;}
	inline float clamp01   (float value)        { return clamp(0.0f, value, 1.0f);}

	// vector operations
    inline float length   (const sVec2& v)                   { return sqrt(square(v.x) + square(v.y));}
    inline float length   (const sVec3& v)                   { return sqrt(square(v.x) + square(v.y) + square(v.z));}
    inline float length   (const sVec4& v)                   { return sqrt(square(square(v.x) + square(v.y) + square(v.z) + square(v.w)));}
	inline float lengthSqr(const sVec2& v)                   { return square(v.x) + square(v.y);}
	inline float lengthSqr(const sVec3& v)                   { return square(v.x) + square(v.y) + square(v.z);}
	inline float lengthSqr(const sVec4& v)                   { return square(v.x) + square(v.y) + square(v.z) + square(v.w);}
	inline sVec2 normalize(const sVec2& v)                   { return v / length(v);}
	inline sVec3 normalize(const sVec3& v)                   { return v / length(v);}
	inline sVec4 normalize(const sVec4& v)                   { return v / length(v);}
	inline float dot      (const sVec2& v1, const sVec2& v2) { return v1.x * v2.x + v1.y * v2.y;}
	inline float dot      (const sVec3& v1, const sVec3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;}
	inline float dot      (const sVec4& v1, const sVec4& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;}
	inline sVec3 cross    (const sVec3& v1, const sVec3& v2) { return {v1.y * v2.z - v2.y * v1.z, v1.z * v2.x - v2.z * v1.x, v1.x * v2.y - v2.x * v1.y};}

	// common matrix construction
	inline sMat4 scale    (float x, float y, float z) { return sMat4(x, y, z, 1.0f);}
	inline sMat4 translate(float x, float y, float z) { sMat4 result(1.0f); result[3][0]=x;result[3][1]=y;result[3][2]=z; return result;}
	inline sMat4 scale    (const sVec3& v)            { return scale(v.x, v.y, v.z);}
	inline sMat4 translate(const sVec3& v)            { return translate(v.x, v.y, v.z);}
	sMat4        rotate   (float angle, sVec3 v);
	sMat4        rotation_translation_scale(sVec4& q, sVec3& t, sVec3& s);

	// matrix operations
	inline sMat4 transpose(const sMat4& m) { sMat4 mr(0.0f); for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) mr[i][j] = m[j][i]; return mr;}
	sMat4        invert   (const sMat4& m);
	sMat4        create_matrix(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33
	);

}

// end of header file
#endif // SEMPER_MATH_H

#ifdef SEMPER_MATH_IMPLEMENTATION

sMat4
Semper::rotation_translation_scale(sVec4& q, sVec3& t, sVec3& s)
{

	// Quaternion math
	float x = q[0];
	float y = q[1];
	float z = q[2];
	float w = q[3];
	float x2 = x + x;
	float y2 = y + y;
	float z2 = z + z;
	float xx = x * x2;
	float xy = x * y2;
	float xz = x * z2;
	float yy = y * y2;
	float yz = y * z2;
	float zz = z * z2;
	float wx = w * x2;
	float wy = w * y2;
	float wz = w * z2;
	float sx = s[0];
	float sy = s[1];
	float sz = s[2];

	sMat4 m = Semper::create_matrix(
		(1.0f - (yy + zz)) * sx,
		(xy - wz) * sy,
		(xz + wy) * sz,
		t[0],
		(xy + wz) * sx,
		(1 - (xx + zz)) * sy,
		(yz - wx) * sz,
		t[1],
		(xz - wy) * sx,
		(yz + wx) * sy,
		(1.0f - (xx + yy)) * sz,
		t[2],
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);


	return m;
}

sMat4 
Semper::rotate(float angle, sVec3 v)
{
	const float a = angle;
	const float c = cos(a);
	const float s = sin(a);

	sVec3 axis = normalize(v);
	sVec3 temp = axis * (1.0f - c);

	sMat4 m = sMat4(1.0f);
	sMat4 rotate{};

	rotate[0][0] = c + temp.x * axis.x;
	rotate[0][1] = temp.x * axis.y + s * axis.z;
	rotate[0][2] = temp.x * axis.z - s * axis.y;

	rotate[1][0] = temp.y * axis.x - s * axis.z;
	rotate[1][1] = c + temp.y * axis.y;
	rotate[1][2] = temp.y * axis.z + s * axis.x;

	rotate[2][0] = temp.z * axis.x + s * axis.y;
	rotate[2][1] = temp.z * axis.y - s * axis.x;
	rotate[2][2] = c + temp.z * axis.z;

	sMat4 result{};
	result[0] = m[0] * rotate[0][0] + m[1] * rotate[0][1] + m[2] * rotate[0][2];
	result[1] = m[0] * rotate[1][0] + m[1] * rotate[1][1] + m[2] * rotate[1][2];
	result[2] = m[0] * rotate[2][0] + m[1] * rotate[2][1] + m[2] * rotate[2][2];
	result[3] = m[3];
	
	return result;
}

sMat4
Semper::invert(const sMat4& m)
{
	sMat4 invout(1.0f);
	float det = 0.0f;

	float a00 = m.at(0);
	float a01 = m.at(1);
	float a02 = m.at(2);
	float a03 = m.at(3);
	float a10 = m.at(4);
	float a11 = m.at(5);
	float a12 = m.at(6);
	float a13 = m.at(7);
	float a20 = m.at(8);
	float a21 = m.at(9);
	float a22 = m.at(10);
	float a23 = m.at(11);
	float a30 = m.at(12);
	float a31 = m.at(13);
	float a32 = m.at(14);
	float a33 = m.at(15);

	float b00 = a00 * a11 - a01 * a10;
	float b01 = a00 * a12 - a02 * a10;
	float b02 = a00 * a13 - a03 * a10;
	float b03 = a01 * a12 - a02 * a11;
	float b04 = a01 * a13 - a03 * a11;
	float b05 = a02 * a13 - a03 * a12;
	float b06 = a20 * a31 - a21 * a30;
	float b07 = a20 * a32 - a22 * a30;
	float b08 = a20 * a33 - a23 * a30;
	float b09 = a21 * a32 - a22 * a31;
	float b10 = a21 * a33 - a23 * a31;
	float b11 = a22 * a33 - a23 * a32;

	det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

	if (det == 0)
	{
		S_MATH_ASSERT(false);
		return invout;
	}

	det = 1.0f / det;

	invout.at(0) = (a11 * b11 - a12 * b10 + a13 * b09) * det;
	invout.at(1) = (a02 * b10 - a01 * b11 - a03 * b09) * det;
	invout.at(2) = (a31 * b05 - a32 * b04 + a33 * b03) * det;
	invout.at(3) = (a22 * b04 - a21 * b05 - a23 * b03) * det;
	invout.at(4) = (a12 * b08 - a10 * b11 - a13 * b07) * det;
	invout.at(5) = (a00 * b11 - a02 * b08 + a03 * b07) * det;
	invout.at(6) = (a32 * b02 - a30 * b05 - a33 * b01) * det;
	invout.at(7) = (a20 * b05 - a22 * b02 + a23 * b01) * det;
	invout.at(8) = (a10 * b10 - a11 * b08 + a13 * b06) * det;
	invout.at(9) = (a01 * b08 - a00 * b10 - a03 * b06) * det;
	invout.at(10) = (a30 * b04 - a31 * b02 + a33 * b00) * det;
	invout.at(11) = (a21 * b02 - a20 * b04 - a23 * b00) * det;
	invout.at(12) = (a11 * b07 - a10 * b09 - a12 * b06) * det;
	invout.at(13) = (a00 * b09 - a01 * b07 + a02 * b06) * det;
	invout.at(14) = (a31 * b01 - a30 * b03 - a32 * b00) * det;
	invout.at(15) = (a20 * b03 - a21 * b01 + a22 * b00) * det;

	return invout;
}

sMat4
Semper::create_matrix(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	sMat4 m(0.0f);

	// column 0
	m[0][0] = m00;
	m[0][1] = m10;
	m[0][2] = m20;
	m[0][3] = m30;

	// column 1
	m[1][0] = m01;
	m[1][1] = m11;
	m[1][2] = m21;
	m[1][3] = m31;

	// column 2
	m[2][0] = m02;
	m[2][1] = m12;
	m[2][2] = m22;
	m[2][3] = m32;

	// column 3
	m[3][0] = m03;
	m[3][1] = m13;
	m[3][2] = m23;
	m[3][3] = m33;

	return m;
}

sMat4
operator*(sMat4 left, sMat4 right)
{
	sVec4 SrcA0 = left[0];
	sVec4 SrcA1 = left[1];
	sVec4 SrcA2 = left[2];
	sVec4 SrcA3 = left[3];

	sVec4 SrcB0 = right[0];
	sVec4 SrcB1 = right[1];
	sVec4 SrcB2 = right[2];
	sVec4 SrcB3 = right[3];

	sMat4 result{};

	result[0] = SrcA0 * SrcB0.x + SrcA1 * SrcB0.y + SrcA2 * SrcB0.z + SrcA3 * SrcB0.w;
	result[1] = SrcA0 * SrcB1.x + SrcA1 * SrcB1.y + SrcA2 * SrcB1.z + SrcA3 * SrcB1.w;
	result[2] = SrcA0 * SrcB2.x + SrcA1 * SrcB2.y + SrcA2 * SrcB2.z + SrcA3 * SrcB2.w;
	result[3] = SrcA0 * SrcB3.x + SrcA1 * SrcB3.y + SrcA2 * SrcB3.z + SrcA3 * SrcB3.w;

	return result;
}

sVec4
operator*(sMat4 left, sVec4 right)
{
	sVec4 Mov0 = { right.x, right.x, right.x, right.x };
	sVec4 Mov1 = { right.y, right.y, right.y, right.y };
	sVec4 Mul0 = left[0] * Mov0;
	sVec4 Mul1 = left[1] * Mov1;
	sVec4 Add0 = Mul0 + Mul1;
	sVec4 Mov2 = { right.z, right.z, right.z, right.z };
	sVec4 Mov3 = { right.w, right.w, right.w, right.w };
	sVec4 Mul2 = left[2] * Mov2;
	sVec4 Mul3 = left[3] * Mov3;
	sVec4 Add1 = Mul2 + Mul3;
	sVec4 Add2 = Add0 + Add1;
	return Add2;
}

sVec3
operator*(sMat4 left, sVec3 right)
{
	sVec4 Mov0 = { right.x, right.x, right.x, right.x };
	sVec4 Mov1 = { right.y, right.y, right.y, right.y };
	sVec4 Mul0 = left[0] * Mov0;
	sVec4 Mul1 = left[1] * Mov1;
	sVec4 Add0 = Mul0 + Mul1;
	sVec4 Mov2 = { right.z, right.z, right.z, right.z };
	sVec4 Mov3 = { 1.0f, 1.0f, 1.0f, 1.0f };
	sVec4 Mul2 = left[2] * Mov2;
	sVec4 Mul3 = left[3] * Mov3;
	sVec4 Add1 = Mul2 + Mul3;
	sVec4 Add2 = Add0 + Add1;
	return { Add2.x, Add2.y, Add2.z };
}

#endif // SEMPER_MATH_IMPLEMENTATION
