#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <cmath>
#include <cfloat>

#include "vec.h"

#include "json/include/nlohmann/json.hpp"

#define M_PI 3.14159263f

struct Ray {
    Vector3f o, d;
    float t = 1e30f;
    float tmax = 1e30f;


    Ray(Vector3f origin, Vector3f direction, float t = 1e30f, float tmax = 1e30f)
        : o(origin), d(direction), t(t), tmax(tmax) {};
};

struct Interaction {
    Vector3f p, n;
    float t = 1e30f;
    bool didIntersect = false;
};
// AAB struct
struct AABB {
    Vector3f min;
    Vector3f max;


    void expand(const Vector3f& point) {
        min = Vector3f(std::min(min.x, point.x), std::min(min.y, point.y), std::min(min.z, point.z));
        max = Vector3f(std::max(max.x, point.x), std::max(max.y, point.y), std::max(max.z, point.z));
    }

    // Expand the AABB to include another AABB
    void expand(const AABB& other) {
        expand(other.min);
        expand(other.max);
    }

    
    
};

class Triangle
{
public:
    Vector3f v1, v2, v3;  // Vertices of the triangle

    // Constructor
    Triangle(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
        : v1(v1), v2(v2), v3(v3)
    {
    }
};







