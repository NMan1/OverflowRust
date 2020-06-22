#pragma once
#include <cmath>
#include <algorithm>

#define M_PI	3.14159265358979323846264338327950288419716939937510

struct matrix4x4 {
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		}; float m[4][4];
	};
};

template <typename t> static  t clamp_value(t value, t min, t max) {
	if (value > max) {
		return max;
	}
	if (value < min) {
		return min;
	}
	return value;
}

//vec_t
class vec_t
{
public:
	vec_t() : x(0.f), y(0.f)
	{

	}

	vec_t(float _x, float _y) : x(_x), y(_y)
	{

	}
	~vec_t()
	{

	}

	bool operator==(vec_t& in) {
		return (this->x == in.x && this->y == in.y);
	}

	float x;
	float y;
};

//vec3_t
class vec3_t
{
public:
	vec3_t() : x(0.f), y(0.f), z(0.f)
	{

	}

	vec3_t(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{

	}
	~vec3_t()
	{

	}

	float x;
	float y;
	float z;

	inline float Dot(vec3_t v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Distance(vec3_t v)
	{
		return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
	}
	inline float Length()
	{
		float ls = x * x + y * y + z * z;
		return sqrt(ls);
	}


	bool operator==(const vec3_t& in) const {
		return (this->x == in.x && this->y == in.y && this->z == in.z);
	}

	vec3_t operator+(vec3_t v)
	{
		return vec3_t(x + v.x, y + v.y, z + v.z);
	}

	vec3_t operator-(vec3_t v)
	{
		return vec3_t(x - v.x, y - v.y, z - v.z);
	}

	vec3_t operator*(float number) const {
		return vec3_t(x * number, y * number, z * number);
	}
	
	vec3_t& operator/=(float fl) {
		x /= fl;
		y /= fl;
		z /= fl;
		return *this;
	}

	vec3_t& operator-=(const vec3_t& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}


	void clamp()
	{
		if (x > 75.f) x = 75.f;
		else if (x < -75.f) x = -75.f;
		if (z < -180) z += 360.0f;
		else if (z > 180) z -= 360.0f;

		y = 0.f;
	}

};

//vec4_t
class vec4_t
{
public:
	vec4_t() : x(0.f), y(0.f), z(0.f), w(0.f)
	{

	}

	vec4_t(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
	{

	}
	~vec4_t()
	{

	}

	float x;
	float y;
	float z;
	float w;
};