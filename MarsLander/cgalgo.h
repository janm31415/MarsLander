#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>

#define pi 3.1415926535897f

#define W 7000
#define H 3000

template <typename T>
struct vec2
{
T x, y;

typedef T value_type;

vec2();

vec2(const T& t);

vec2(const T& _x, const T& _y);

template <typename T2>
T& operator [] (T2 i);

template <typename T2>
const T& operator [] (T2 i) const;

};

template <typename T>
vec2<T>::vec2() {}

template <typename T>
vec2<T>::vec2(const T& t) : x(t), y(t) {}

template <typename T>
vec2<T>::vec2(const T& _x, const T& _y) : x(_x), y(_y) {}

template <typename T>
template <typename T2>
T& vec2<T>::operator [] (T2 i) { return (&x)[i]; }

template <typename T>
template <typename T2>
const T& vec2<T>::operator [] (T2 i) const { return (&x)[i]; }

template <typename T>
inline vec2<T> operator + (const vec2<T>& a)
{
return vec2<T>(+a.x, +a.y);
}

template <typename T>
inline vec2<T> operator - (const vec2<T>& a)
{
return vec2<T>(-a.x, -a.y);
}

template <typename T>
inline vec2<T> operator + (const vec2<T>& left, const vec2<T>& right)
{
return vec2<T>(left.x + right.x, left.y + right.y);
}

template <typename T>
inline vec2<T> operator - (const vec2<T>& left, const vec2<T>& right)
{
return vec2<T>(left.x - right.x, left.y - right.y);
}

template <typename T>
inline vec2<T> operator * (const vec2<T>& left, const vec2<T>& right)
{
return vec2<T>(left.x * right.x, left.y * right.y);
}

template <typename T>
inline vec2<T> operator * (const T& s, const vec2<T>& right)
{
return vec2<T>(s * right.x, s * right.y);
}

template <typename T>
inline vec2<T> operator * (const vec2<T>& left, const T& s)
{
return vec2<T>(left.x * s, left.y * s);
}

template <typename T>
inline vec2<T> operator / (const vec2<T>& left, const vec2<T>& right)
{
return vec2<T>(left.x / right.x, left.y / right.y);
}

template <typename T>
inline vec2<T> operator / (const T& s, const vec2<T>& right)
{
return vec2<T>(s / right.x, s / right.y);
}

template <typename T>
inline vec2<T> operator / (const vec2<T>& left, const T& s)
{
return vec2<T>(left.x / s, left.y / s);
}

template <typename T>
inline vec2<T> min(const vec2<T>& left, const vec2<T>& right)
{
return vec2<T>(min(left.x, right.x), min(left.y, right.y));
}

template <typename T>
inline vec2<T> max(const vec2<T>& left, const vec2<T>& right)
{
return vec2<T>(max(left.x, right.x), max(left.y, right.y));
}

template <typename T>
inline vec2<T> abs(const vec2<T>& a)
{
return vec2<T>(abs(a.x), abs(a.y));
}

template <>
inline vec2<float> abs(const vec2<float>& a)
{
return vec2<float>(fabs(a.x), fabs(a.y));
}

template <typename T>
inline vec2<T> sqrt(const vec2<T>& a)
{
return vec2<T>(sqrt(a.x), sqrt(a.y));
}

template <typename T>
inline vec2<T> rsqrt(const vec2<T>& a)
{
return vec2<T>(rsqrt(a.x), rsqrt(a.y));
}

template <typename T>
inline vec2<T> reciprocal(const vec2<T>& a)
{
return vec2<T>(reciprocal(a.x), reciprocal(a.y));
}

template <typename T>
inline T dot(const vec2<T>& left, const vec2<T>& right)
{
return left.x*right.x + left.y*right.y;
}

template <typename T>
inline T length(const vec2<T>& a)
{
return sqrt(dot(a, a));
}

template <>
inline float length(const vec2<float>& a)
{
return std::sqrt(dot(a, a));
}

template <>
inline double length(const vec2<double>& a)
{
return std::sqrt(dot(a, a));
}

template <typename T>
inline T distance(const vec2<T>& left, const vec2<T>& right)
{
return length(left - right);
}

template <typename T>
inline T distance_sqr(const vec2<T>& left, const vec2<T>& right)
{
auto a = (left - right);
return dot(a, a);
}

template <typename T>
inline vec2<T> normalize(const vec2<T>& a)
{
return a * rsqrt(dot(a, a));
}

template <>
inline vec2<float> normalize(const vec2<float>& a)
{
return a / std::sqrt(dot(a, a));
}

template <>
inline vec2<double> normalize(const vec2<double>& a)
{
return a / std::sqrt(dot(a, a));
}

template <typename T>
inline bool operator < (const vec2<T>& left, const vec2<T>& right)
{
return (left[0] == right[0]) ? (left[1] < right[1]) : (left[0] < right[0]);
}

template <typename T>
inline bool operator > (const vec2<T>& left, const vec2<T>& right)
{
return (left[0] == right[0]) ? (left[1] > right[1]) : (left[0] > right[0]);
}

template <typename T>
inline bool operator == (const vec2<T>& left, const vec2<T>& right)
{
return (left[0] == right[0]) && (left[1] == right[1]);
}

template <typename T>
inline bool operator != (const vec2<T>& left, const vec2<T>& right)
{
return !(left == right);
}

struct sdata {
    vec2<float> p,v;
    int F; // the quantity of remaining fuel in liters.
    int R; // the rotation angle in degrees (-90 to 90).
    int P; // the thrust power (0 to 4).
};


extern std::vector<vec2<int>> surface_points;

void read_input(std::stringstream& strcin, std::stringstream& strerr);
void do_main(std::stringstream& strcin, std::stringstream& strerr);
