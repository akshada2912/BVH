#include "scene.h"

Scene::Scene(std::string sceneDirectory, std::string sceneJson)
{
    nlohmann::json sceneConfig;
    try
    {
        sceneConfig = nlohmann::json::parse(sceneJson);
    }
    catch (std::runtime_error e)
    {
        std::cerr << "Could not parse json." << std::endl;
        exit(1);
    }

    this->parse(sceneDirectory, sceneConfig);
}

Scene::Scene(std::string pathToJson)
{
    std::string sceneDirectory;

#ifdef _WIN32
    const size_t last_slash_idx = pathToJson.rfind('\\');
#else
    const size_t last_slash_idx = pathToJson.rfind('/');
#endif

    if (std::string::npos != last_slash_idx)
    {
        sceneDirectory = pathToJson.substr(0, last_slash_idx);
    }

    nlohmann::json sceneConfig;
    try
    {
        std::ifstream sceneStream(pathToJson.c_str());
        sceneStream >> sceneConfig;
    }
    catch (std::runtime_error e)
    {
        std::cerr << "Could not load scene .json file." << std::endl;
        exit(1);
    }

    this->parse(sceneDirectory, sceneConfig);
}

void Scene::parse(std::string sceneDirectory, nlohmann::json sceneConfig)
{
    // Output
    try
    {
        auto res = sceneConfig["output"]["resolution"];
        this->imageResolution = Vector2i(res[0], res[1]);
    }
    catch (nlohmann::json::exception e)
    {
        std::cerr << "\"output\" field with resolution, filename & spp should be defined in the scene file." << std::endl;
        exit(1);
    }

    // Cameras
    try
    {
        auto cam = sceneConfig["camera"];

        this->camera = Camera(
            Vector3f(cam["from"][0], cam["from"][1], cam["from"][2]),
            Vector3f(cam["to"][0], cam["to"][1], cam["to"][2]),
            Vector3f(cam["up"][0], cam["up"][1], cam["up"][2]),
            float(cam["fieldOfView"]),
            this->imageResolution);
    }
    catch (nlohmann::json::exception e)
    {
        std::cerr << "No camera(s) defined. Atleast one camera should be defined." << std::endl;
        exit(1);
    }

    // Surface
    try
    {
        auto surfacePaths = sceneConfig["surface"];

        uint32_t surfaceIdx = 0;
        for (std::string surfacePath : surfacePaths)
        {
            surfacePath = sceneDirectory + "/" + surfacePath;

            auto surf = createSurfaces(surfacePath, /*isLight=*/false, /*idx=*/surfaceIdx);
            this->surfaces.insert(this->surfaces.end(), surf.begin(), surf.end());

            // for (const Surface &singleSurface : surf)
            // {
            //     AABB surfaceAABB = singleSurface.computeAABB();
            //     this->surfaceAABBs.push_back(surfaceAABB);
            // }
            // this->bvhRoot = buildBVHRecursive(this->surfaces, 0, this->surfaces.size());

            surfaceIdx = surfaceIdx + surf.size();
        }
    }
    catch (nlohmann::json::exception e)
    {
        std::cout << "No surfaces defined." << std::endl;
    }
}

Interaction Scene::rayIntersect(Ray &ray)
{
    // ray = this->camera.transformRayToWorld(ray);
    Interaction siFinal;

    for (auto &surface : this->surfaces)
    {
        Interaction si = surface.rayIntersect(ray);
        if (si.t <= ray.t)
        {
            siFinal = si;
            ray.t = si.t;
        }
    }

    // siFinal.p = this->camera.transformPointToWorld(siFinal.p);
    return siFinal;
}

// Function to test whether a given ray intersects a given AABB
bool aabb_intersect(const Ray &ray, const AABB &aabb)
{
    // intersection of ray w AABB along x axis
    float tmin = (aabb.min.x - ray.o.x) * 1.0f / ray.d.x;
    float tmax = (aabb.max.x - ray.o.x) * 1.0f / ray.d.x;

    // if (tmin > tmax)
    //     std::swap(tmin, tmax);

    // along y axis
    float tymin = (aabb.min.y - ray.o.y) * 1.0f / ray.d.y;
    float tymax = (aabb.max.y - ray.o.y) * 1.0f / ray.d.y;

    // if (tymin > tymax)
    //     std::swap(tymin, tymax);
    // if ((tmin > tymax) || (tymin > tmax))
    //     return false;

    // if (tymin > tmin)
    //     tmin = tymin;
    // if (tymax < tmax)
    //     tmax = tymax;

    // along z axis
    float tzmin = (aabb.min.z - ray.o.z) * 1.0f / ray.d.z;
    float tzmax = (aabb.max.z - ray.o.z) * 1.0f / ray.d.z;

    // if (tzmin > tzmax)
    //     std::swap(tzmin, tzmax);
    // if ((tmin > tzmax) || (tzmin > tmax))
    //     return false;

    float tminf = std::max(std::max(std::min(tmin, tmax), std::min(tymin, tymax)), std::min(tzmin, tzmax));
    float tmaxf = std::min(std::min(std::max(tmin, tmax), std::max(tymin, tymax)), std::max(tzmin, tzmax));

    return tmaxf > 0 && tminf <= tmaxf;

    // if (tzmin > tmin) tmin = tzmin;
    // if (tzmax < tmax) tmax = tzmax;

    // if ((tmax < tmin) || (tmin < ray.t) || (tmax < 0 )) return false;

    return true;
}

// 2.1 AABBCODE
Interaction Scene::rayIntersect2(Ray &ray)
{
    Interaction siFinal;
    int i = 0;
    for (auto &surface : this->surfaces)
    {
        AABB surfaceAABB = surface.computeAABB();
        this->surfaceAABBs.push_back(surfaceAABB);
    }
    for (auto &surface : this->surfaces)
    {
        if (aabb_intersect(ray, this->surfaceAABBs[i]))
        {
            Interaction si = surface.rayIntersect(ray);
            if (si.t <= ray.t)
            {
                siFinal = si;
                ray.t = si.t;
            }
        }
        i += 1;
    }

    return siFinal;
}

// 2.3 helper

int findLongestAxis(const std::vector<Vector3i> &indices)
{
    // Initialize the extents to a negative value to ensure they get updated
    float maxX = -std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    float maxZ = -std::numeric_limits<float>::infinity();

    // Compute the maximum extents along each axis
    for (const Vector3i &index : indices)
    {
        // Update extents based on index
        maxX = std::max(maxX, static_cast<float>(index.x));
        maxY = std::max(maxY, static_cast<float>(index.y));
        maxZ = std::max(maxZ, static_cast<float>(index.z));
    }

    // Find the axis with the maximum extent
    if (maxX > maxY && maxX > maxZ)
        return 0; // x-axis
    else if (maxY > maxZ)
        return 1; // y-axis
    else
        return 2; // z-axis
}

BVHNode *buildObjectBVH(std::vector<Vector3i> indices, int start, int end)
{
    // printf("*%d ",indices.size());
    if (indices.size() == 0)
        return nullptr;

    BVHNode *node = new BVHNode;
    AABB bbox;

    Vector3f minBound;
    Vector3f maxBound;
    for (const auto &vertex : indices)
    {
        minBound = Vector3f(static_cast<float>(vertex.x), static_cast<float>(vertex.y), static_cast<float>(vertex.z));
        maxBound = Vector3f(static_cast<float>(vertex.x), static_cast<float>(vertex.y), static_cast<float>(vertex.z));
        break;
    }

    // Iterate over each vertex and update min and max values
    for (const auto &vertex : indices)
    {
        if (minBound.x > vertex.x)
        {
            minBound.x = static_cast<float>(vertex.x);
        }
        if (minBound.y > vertex.y)
        {
            minBound.y = static_cast<float>(vertex.y);
        }
        if (minBound.z > vertex.z)
        {
            minBound.z = static_cast<float>(vertex.z);
        }

        if (maxBound.x < vertex.x)
        {
            maxBound.x = static_cast<float>(vertex.x);
        }
        if (maxBound.y < vertex.y)
        {
            maxBound.y = static_cast<float>(vertex.y);
        }
        if (maxBound.z < vertex.z)
        {
            maxBound.z = static_cast<float>(vertex.z);
        }
    }

    bbox.min = minBound;
    bbox.max = maxBound;
    node->boundingBox = bbox;
    printf("**%f %f\n", node->boundingBox.max, node->boundingBox.min);

    // Calculate bounding box for the surface's vertices

    // If there are few vertices or a termination condition is met, create a leaf node
    if (indices.size() <= 1)
    {
        // Assuming you have a copy constructor for Surface
        // node->surface_leaf = new Surface(surfaces);
        node->left = nullptr;
        node->vertexes = indices;
        // printf("%d %d\n",vertex.size(),nodeses.size());
        node->right = nullptr;
        return node;
    }

    // int longestAxis = findLongestAxis(indices);
    // std::sort(indices.begin(), indices.end(), [longestAxis](const Vector3i &a, const Vector3i &b) {
    //     return a[longestAxis] < b[longestAxis];
    // });

    // Otherwise, split vertices and build child nodes
    int mid = indices.size() / 2;
    std::vector<Vector3i> left_vertices(indices.begin(), indices.begin() + mid);
    std::vector<Vector3i> right_vertices(indices.begin() + mid, indices.end());

    node->left = buildObjectBVH(left_vertices, start, mid);
    node->right = buildObjectBVH(right_vertices, mid, end);

    // Update bounding box to encompass both child nodes

    return node;

    // if (surfaces.vertices.size() == 0)
    //     return nullptr;

    // BVHNode* node = new BVHNode;
    // // Calculate bounding box for the surface's vertices

    // // If there are few vertices or a termination condition is met, create a leaf node
    // if (surfaces.vertices.size() <= 3)
    // {
    //     // Assuming you have a copy constructor for Surface
    //     //node->surface_leaf = new Surface(surfaces);
    //     node->left = nullptr;
    //     node->right = nullptr;
    //     return node;
    // }

    // // Otherwise, split vertices and build child nodes
    // int mid = surfaces.vertices.size() / 2;
    // std::vector<Vector3f> left_vertices(surfaces.vertices.begin(), surfaces.vertices.begin() + mid);
    // std::vector<Vector3f> right_vertices(surfaces.vertices.begin() + mid, surfaces.vertices.end());

    // node->left = buildObjectBVH(left_surface,start,mid);
    // node->right = buildObjectBVH(right_surface,mid,end);

    // // Update bounding box to encompass both child nodes

    // return node;
    // printf("h#i");
    // if (start >= end)
    // {
    //     return nullptr;
    // }

    // BVHNode *node = new BVHNode;
    // // Calculate bounding box for surfaces[start:end]
    // AABB nodeAABB;

    // for (int i = 0; i < surfaces.size(); ++i)
    // {
    //     for(int j=0;j<surfaces[i]->vertices.size();j++){
    //         printf("%d %d %d\n",surfaces[i]->vertices[j].x,surfaces[i]->vertices[j].y,surfaces[i]->vertices[j].z);
    //     }
    //     nodeAABB.expand(surfaces[i]->computeAABB());
    // }
    // node->boundingBox = nodeAABB;

    // // If there are few surfaces or a termination condition is met, create a leaf node
    // if (end - start == 1)
    // {
    //     Surface *surface = new Surface(*surfaces[start]); // Assuming you have a copy constructor
    //     node->triangles.push_back(surface);
    //     node->left = nullptr;
    //     node->right = nullptr;
    //     return node;
    // }

    // // Otherwise, split surfaces and build child nodes
    // int mid = (start + end) / 2;
    // node->left = buildObjectBVH(surfaces, start, mid);
    // node->right = buildObjectBVH(surfaces, mid, end);

    // return node;
}

// 2.2 BVH code
BVHNode *Scene::buildBVHRecursive(std::vector<Surface> &surfaces, int start, int end)
{
    if (surfaces.size() == 0)
        return nullptr;
    BVHNode *node = new BVHNode;
    // Calculate bounding box for surfaces[start:end]
    AABB nodeAABB;

    for (int i = 0; i < surfaces.size(); ++i)
    {
        // Surface *surface = new Surface(surfaces[i]); // Assuming you have a copy constructor
        // node->triangles.push_back(surface);
        nodeAABB.expand(surfaces[i].computeAABB());
    }
    node->boundingBox = nodeAABB;
    printf("%f %f\n", node->boundingBox.max, node->boundingBox.min);

    // If there are few surfaces or a termination condition is met, create a leaf node
    if (surfaces.size() <= 1)
    {
        // for (int i = 0; i < end; ++i)
        // {
        //     Surface *surface = new Surface(surfaces[i]); // Assuming you have a copy constructor
        //     node->triangles.push_back(surface);
        // }
        node->triangles.push_back(new Surface(surfaces[0]));
        node->left = nullptr;
        node->right = nullptr;
        node->objectBVH = buildObjectBVH(node->triangles[0]->indices, 0, node->triangles.size());

        return node;
    }
    else
    {
        std::vector<Surface> &axis_div = surfaces;
        if (node->boundingBox.max.x - node->boundingBox.min.x >= std::max(node->boundingBox.max.y - node->boundingBox.min.y, node->boundingBox.max.z - node->boundingBox.min.z))
        {
            std::sort(axis_div.begin(), axis_div.end(), [](Surface &a, Surface &b)
                      {
                float max_a=a.vertices[0].x;
                float min_a=a.vertices[0].x;
                float max_y=a.vertices[0].y;
                float min_y=a.vertices[0].y;
                float max_z=a.vertices[0].z;
                float min_z=a.vertices[0].z;
                for(int i=1;i<a.vertices.size();i++)
                {
                    max_a=std::max(max_a,a.vertices[i].x);
                    min_a=std::min(min_a,a.vertices[i].x);
                    max_y=std::max(max_y,a.vertices[i].y);
                    min_y=std::min(min_y,a.vertices[i].y);
                    max_z=std::max(max_z,a.vertices[i].z);
                    min_z=std::min(min_z,a.vertices[i].z);
                }
                AABB one;
                one.max.x=max_a;
                one.min.x=min_a;
                one.max.y=max_y;
                one.min.y=min_y;
                one.max.z=max_z;
                one.min.z=min_z;

                float max_a2=b.vertices[0].x;
                float min_a2=b.vertices[0].x;
                float max_y2=b.vertices[0].y;
                float min_y2=b.vertices[0].y;
                float max_z2=b.vertices[0].z;
                float min_z2=b.vertices[0].z;
                for(int i=1;i<b.vertices.size();i++)
                {
                    max_a2=std::max(max_a2,b.vertices[i].x);
                    min_a2=std::min(min_a2,b.vertices[i].x);
                    max_y2=std::max(max_y2,b.vertices[i].y);
                    min_y2=std::min(min_y2,b.vertices[i].y);
                    max_z2=std::max(max_z2,b.vertices[i].z);
                    min_z2=std::min(min_z2,b.vertices[i].z);
                }
                AABB two;
                two.max.x=max_a2;
                two.min.x=min_a2;
                two.max.y=max_y2;
                two.min.y=min_y2;
                two.max.z=max_z2;
                two.min.z=min_z2;

                return one.min.x<two.min.x; });
        }
        else if (node->boundingBox.max.y - node->boundingBox.min.y >= std::max(node->boundingBox.max.x - node->boundingBox.min.x, node->boundingBox.max.z - node->boundingBox.min.z))
        {
            std::sort(axis_div.begin(), axis_div.end(), [](Surface &a, Surface &b)
                      {
                float max_a=a.vertices[0].x;
                float min_a=a.vertices[0].x;
                float max_y=a.vertices[0].y;
                float min_y=a.vertices[0].y;
                float max_z=a.vertices[0].z;
                float min_z=a.vertices[0].z;
                for(int i=1;i<a.vertices.size();i++)
                {
                    max_a=std::max(max_a,a.vertices[i].x);
                    min_a=std::min(min_a,a.vertices[i].x);
                    max_y=std::max(max_y,a.vertices[i].y);
                    min_y=std::min(min_y,a.vertices[i].y);
                    max_z=std::max(max_z,a.vertices[i].z);
                    min_z=std::min(min_z,a.vertices[i].z);
                }
                AABB one;
                one.max.x=max_a;
                one.min.x=min_a;
                one.max.y=max_y;
                one.min.y=min_y;
                one.max.z=max_z;
                one.min.z=min_z;

                float max_a2=b.vertices[0].x;
                float min_a2=b.vertices[0].x;
                float max_y2=b.vertices[0].y;
                float min_y2=b.vertices[0].y;
                float max_z2=b.vertices[0].z;
                float min_z2=b.vertices[0].z;
                for(int i=1;i<b.vertices.size();i++)
                {
                    max_a2=std::max(max_a2,b.vertices[i].x);
                    min_a2=std::min(min_a2,b.vertices[i].x);
                    max_y2=std::max(max_y2,b.vertices[i].y);
                    min_y2=std::min(min_y2,b.vertices[i].y);
                    max_z2=std::max(max_z2,b.vertices[i].z);
                    min_z2=std::min(min_z2,b.vertices[i].z);
                }
                AABB two;
                two.max.x=max_a2;
                two.min.x=min_a2;
                two.max.y=max_y2;
                two.min.y=min_y2;
                two.max.z=max_z2;
                two.min.z=min_z2;

                return one.min.y<two.min.y; });
        }
        else
        {
            std::sort(axis_div.begin(), axis_div.end(), [](Surface &a, Surface &b)
                      {
                float max_a=a.vertices[0].x;
                float min_a=a.vertices[0].x;
                float max_y=a.vertices[0].y;
                float min_y=a.vertices[0].y;
                float max_z=a.vertices[0].z;
                float min_z=a.vertices[0].z;
                for(int i=1;i<a.vertices.size();i++)
                {
                    max_a=std::max(max_a,a.vertices[i].x);
                    min_a=std::min(min_a,a.vertices[i].x);
                    max_y=std::max(max_y,a.vertices[i].y);
                    min_y=std::min(min_y,a.vertices[i].y);
                    max_z=std::max(max_z,a.vertices[i].z);
                    min_z=std::min(min_z,a.vertices[i].z);
                }
                AABB one;
                one.max.x=max_a;
                one.min.x=min_a;
                one.max.y=max_y;
                one.min.y=min_y;
                one.max.z=max_z;
                one.min.z=min_z;

                float max_a2=b.vertices[0].x;
                float min_a2=b.vertices[0].x;
                float max_y2=b.vertices[0].y;
                float min_y2=b.vertices[0].y;
                float max_z2=b.vertices[0].z;
                float min_z2=b.vertices[0].z;
                for(int i=1;i<b.vertices.size();i++)
                {
                    max_a2=std::max(max_a2,b.vertices[i].x);
                    min_a2=std::min(min_a2,b.vertices[i].x);
                    max_y2=std::max(max_y2,b.vertices[i].y);
                    min_y2=std::min(min_y2,b.vertices[i].y);
                    max_z2=std::max(max_z2,b.vertices[i].z);
                    min_z2=std::min(min_z2,b.vertices[i].z);
                }
                AABB two;
                two.max.x=max_a2;
                two.min.x=min_a2;
                two.max.y=max_y2;
                two.min.y=min_y2;
                two.max.z=max_z2;
                two.min.z=min_z2;

                return one.min.z<two.min.z; });
        }
        int mid = (axis_div.size()) / 2;
        auto mid_iter = axis_div.begin() + mid;
        std::vector<Surface> left_list(axis_div.begin(), mid_iter);
        std::vector<Surface> right_list(mid_iter, axis_div.end());

        // Otherwise, split surfaces and build child nodes
        // int mid = (surfaces.size()) / 2;
        // auto mid_iter = surfaces.begin() + mid;
        // std::vector<Surface> left_list(surfaces.begin(), mid_iter);
        // std::vector<Surface> right_list(mid_iter, surfaces.end());

        node->left = buildBVHRecursive(left_list, start, mid);
        node->right = buildBVHRecursive(right_list, mid, end);

        // Update bounding box to encompass both child nodes

        return node;
        // surfaces=axis_div;
    }
}

Interaction Scene::rayIntersectBVH(Ray &ray, BVHNode *node)
{
    Interaction siFinal;
    if (node == nullptr)
        return Interaction();
    // auto startTime2 = std::chrono::high_resolution_clock::now();
    if (aabb_intersect(ray, node->boundingBox))
    {
        if (node->left == nullptr && node->right == nullptr)
        {
            // Leaf node: Check intersections with individual triangles
            Interaction si = node->triangles[0]->rayIntersect(ray);
            if (si.t <= ray.t)
            {
                siFinal = si;
                ray.t = si.t;
            }
            // for (auto &triangle : node->triangles)
            // {

            //     Interaction si = triangle->rayIntersect(ray);
            //     if (si.t <= ray.t)
            //     {
            //         siFinal = si;
            //         ray.t = si.t;
            //     }
            // }
        }
        else
        {
            // Internal node: Recursively check child nodes
            Interaction siLeft;
            Interaction siRight;
            if (aabb_intersect(ray, node->left->boundingBox))
                siLeft = rayIntersectBVH(ray, node->left);
            else
            {
                siLeft.didIntersect = 0;
            }
            if (aabb_intersect(ray, node->right->boundingBox))
                siRight = rayIntersectBVH(ray, node->right);
            else
            {
                siRight.didIntersect = 0;
            }
            // siRight = rayIntersectBVH(ray, node->right);

            if (siLeft.didIntersect && siRight.didIntersect)
            {
                // Both left and right nodes have intersections
                if (siLeft.t < siRight.t)
                {
                    siFinal = siLeft;
                }
                else
                {
                    siFinal = siRight;
                }
            }
            else if (siLeft.didIntersect)
            {
                // Only left node has an intersection
                siFinal = siLeft;
            }
            else if (siRight.didIntersect)
            {
                // Only right node has an intersection
                siFinal = siRight;
            }
        }
    }
    // else{
    //     printf("no");
    // }

    // auto finishTime = std::chrono::high_resolution_clock::now();
    // auto renderTime = std::chrono::duration_cast<std::chrono::microseconds>(startTime2 - startTime2).count();
    // std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    return siFinal;
}

// 2.3: BVH ON TRIANGLES

Interaction Scene::rayIntersectObjectBVH(Ray &ray, BVHNode *node, Surface *triangle1)
{
    Interaction siFinal;
    float tmin = ray.t;
    // printf("%d ", node->vertexes.size());
    for (auto face : node->vertexes)
    {
        Vector3f p1 = triangle1->vertices[face.x];
        Vector3f p2 = triangle1->vertices[face.y];
        Vector3f p3 = triangle1->vertices[face.z];

        Vector3f n1 = triangle1->normals[face.x];
        Vector3f n2 = triangle1->normals[face.y];
        Vector3f n3 = triangle1->normals[face.z];
        Vector3f n = Normalize(n1 + n2 + n3);

        Interaction si = triangle1->rayTriangleIntersect(ray, p1, p2, p3, n);
        if (si.t <= tmin && si.didIntersect)
        {
            siFinal = si;
            tmin = si.t;
        }
    }

    return siFinal;
}

// bool rayIntersectsBox(const Ray& ray, const AABB& box)
// {
//     Vector3f t1 = (box.min_3 - ray.o) / ray.d;
//     Vector3f t2 = (box.max_3 - ray.o) / ray.d;

//     Vector3f tmin = std::min(t1, t2);
//     Vector3f tmax = std::max(t1, t2);

//     float tmin_max = std::max(std::max(tmin.x, tmin.y), tmin.z);
//     float tmax_min = std::min(std::min(tmax.x, tmax.y), tmax.z);

//     return tmin_max <= tmax_min;
// }

Interaction Scene::rayIntersectBVHSecond(Ray &ray, BVHNode *node, Surface *triangle1)
{
    Interaction siFinal;
    if (aabb_intersect(ray, node->boundingBox))
    {
        // printf("%d\n",node->vertexes[0].x);
        if (node->left == nullptr && node->right == nullptr)
        {
            Interaction si = rayIntersectObjectBVH(ray, node, triangle1);
            // Interaction si=node->triangles[0]->rayIntersect(ray);
            if (si.t <= ray.t)
            {
                siFinal = si;
                ray.t = si.t;
            }
            // Leaf node: Check intersections with individual triangles
            // for (auto &triangle : node->triangles)
            // {

            //     Interaction si = triangle->rayIntersectObjectBVH(ray);
            //     if (si.t <= ray.t)
            //     {
            //         siFinal = si;
            //         ray.t = si.t;
            //     }
            // }
        }
        else
        {
            // Internal node: Recursively check child nodes
            Interaction siLeft;
            Interaction siRight;
            siLeft = rayIntersectBVHSecond(ray, node->left, triangle1);
            siRight = rayIntersectBVHSecond(ray, node->right, triangle1);

            if (siLeft.didIntersect && siRight.didIntersect)
            {
                // Both left and right nodes have intersections
                if (siLeft.t < siRight.t)
                {
                    siFinal = siLeft;
                }
                else
                {
                    siFinal = siRight;
                }
            }
            else if (siLeft.didIntersect)
            {
                // Only left node has an intersection
                siFinal = siLeft;
            }
            else if (siRight.didIntersect)
            {
                // Only right node has an intersection
                siFinal = siRight;
            }
        }
    }
    // else{
    //     printf("no");
    // }

    // auto finishTime = std::chrono::high_resolution_clock::now();
    // auto renderTime = std::chrono::duration_cast<std::chrono::microseconds>(startTime2 - startTime2).count();
    // std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    return siFinal;
}

Interaction Scene::rayIntersectBVHLeaf(Ray &ray, BVHNode *node)
{
    Interaction siFinal;
    // auto startTime2 = std::chrono::high_resolution_clock::now();
    if (aabb_intersect(ray, node->boundingBox))
    {
        if (node->left == nullptr && node->right == nullptr)
        {
            Interaction si = rayIntersectBVHSecond(ray, node->objectBVH, node->triangles[0]);
            // Interaction si=node->triangles[0]->rayIntersect(ray);
            if (si.t <= ray.t)
            {
                siFinal = si;
                ray.t = si.t;
            }
            // Leaf node: Check intersections with individual triangles
            // for (auto &triangle : node->triangles)
            // {

            //     Interaction si = triangle->rayIntersectObjectBVH(ray);
            //     if (si.t <= ray.t)
            //     {
            //         siFinal = si;
            //         ray.t = si.t;
            //     }
            // }
        }
        else
        {
            // Internal node: Recursively check child nodes
            Interaction siLeft;
            Interaction siRight;
            siLeft = rayIntersectBVHLeaf(ray, node->left);
            siRight = rayIntersectBVHLeaf(ray, node->right);

            if (siLeft.didIntersect && siRight.didIntersect)
            {
                // Both left and right nodes have intersections
                if (siLeft.t < siRight.t)
                {
                    siFinal = siLeft;
                }
                else
                {
                    siFinal = siRight;
                }
            }
            else if (siLeft.didIntersect)
            {
                // Only left node has an intersection
                siFinal = siLeft;
            }
            else if (siRight.didIntersect)
            {
                // Only right node has an intersection
                siFinal = siRight;
            }
        }
    }
    // else{
    //     printf("no");
    // }

    // auto finishTime = std::chrono::high_resolution_clock::now();
    // auto renderTime = std::chrono::duration_cast<std::chrono::microseconds>(startTime2 - startTime2).count();
    // std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    return siFinal;
}
