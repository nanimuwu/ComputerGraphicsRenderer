#pragma once
#include "Object3DRenderer.h"
#include "RayTriangleIntersection.h"

struct IntersectionResult {
    RayTriangleIntersection targetTriangle;
    glm::vec3 barycentric;
};

class RayTracedRenderer: public Object3DRenderer {
    public:
        void draw(DrawingWindow &window) override;
        void setRenderType(rendererType renderType);
        void toggleGouraud();
        void togglePhong();
    private:
        IntersectionResult getClosestIntersection(glm::vec3 startingPoint, glm::vec3 rayDirection);
        void updateTriangleVertexInfo(RayTriangleIntersection &targetTriangle);
};
