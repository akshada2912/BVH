# Basic Bounding Volume Hierarchy (BVH) 
In the base implementation given, the ray-object intersection tests are implemented naıvely by iterating over all of the objects (listing 1) and then iterating over all triangles (listing 2) of the object and finding the closest intersection point. The runtime of this algorithm increases linearly (O(n)) with the number of triangles in the scene, making the method unscalable for a large number of triangles.

The idea of Bounding Volume Hierarchy (BVH) is to avoid this linear search and instead build a binary tree over triangles. Doing so brings the runtime to logarithmic in the number of triangles of the scene (O(log(n))).

1. Axis Aligned Bounding Boxes on Surfaces 

Used axis-aligned bounding boxes (AABBs) to accelerate the intersection tests. Modified the code to do the following things:
• Create Axis-Aligned Bounding Boxes (AABBs) for each object and write a function to test whether a given ray intersects a given AABB.
• Iterate over all the bounding boxes of objects in the scene and test intersection with them.
• If the ray intersects the bounding box, iterate over all triangles of the object to find the closest intersection.

2. BVH on bounding boxes 

We will accelerate the intersection tests in this question by building a Bounding Volume Hierarchy (BVH) over the AABBs. Modified the code to do the following:
• Write code to create a BVH on the bounding boxes.
• Traverse the BVH recursively.
• If the ray intersects the AABB at the leaf node, iterate over all triangles of the object to find the closest intersection.


3. BVH on triangles (Two-Level BVH)
Finally, I built a two-level BVH to optimize the intersection queries further. Modified the code to do the following.
• Build a separate BVH for each object.
• Traverse the top-level BVH recursively.
• If the ray intersects the bounding box at the leaf node, then traverse the BVH of that object to find the closest intersection point.
