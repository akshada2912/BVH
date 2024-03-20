#pragma once

#include "camera.h"
#include "surface.h"

struct BVHNode {
    AABB boundingBox;
    BVHNode* left;
    BVHNode* right;
    std::vector<Surface*> triangles;
    std::vector<AABB> AABBs;
    std::vector<Vector3i> vertexes;
    std::vector<Vector3f> normals;
    BVHNode* objectBVH;

    // ~BVHNode() {
    //     // Clean up allocated memory when BVHNode is destructed
    //     for (Surface* surface : triangles) {
    //         delete surface;
    //     }
    // }

};

struct Scene {
    std::vector<Surface> surfaces;
    std::vector<AABB> surfaceAABBs;
    std::vector<Triangle> triangles;
    Camera camera;
    Vector2i imageResolution;

    Scene() {};
    Scene(std::string sceneDirectory, std::string sceneJson);
    Scene(std::string pathToJson);
    
    void parse(std::string sceneDirectory, nlohmann::json sceneConfig);

    Interaction rayIntersect(Ray& ray);
    Interaction rayIntersect2(Ray& ray);

    Interaction rayIntersectBVH(Ray& ray, BVHNode* node);
    Interaction rayIntersectObjectBVH(Ray& ray, BVHNode* node,Surface* triangle1);
    Interaction rayIntersectBVHLeaf(Ray& ray, BVHNode* node);
    
    Interaction rayIntersectBVHSecond(Ray& ray, BVHNode* node, Surface* triangle1);
    BVHNode* buildBVHRecursive(std::vector<Surface>& surfaces, int start, int end);
    


    BVHNode* bvhRoot;
};

