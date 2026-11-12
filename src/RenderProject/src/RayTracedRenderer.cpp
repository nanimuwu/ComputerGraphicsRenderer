#include "RayTracedRenderer.h"
#include <glm/glm.hpp>

RayTriangleIntersection RayTracedRenderer::getClosestIntersection(glm::vec3 rayDirection){
    
    std::vector<RayTriangleIntersection> output;

    for (int i = 0; i<triangles.size(); i++) {
        glm::vec3 e0 = triangles[i].vertices[1] - triangles[i].vertices[0];
        glm::vec3 e1 = triangles[i].vertices[2] - triangles[i].vertices[0];
        glm::vec3 SPVector = cameraPosition - triangles[i].vertices[0];
        glm::mat3 DEMatrix(-rayDirection, e0, e1);
        glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
        if (output.size() == 0){
            output.push_back(RayTriangleIntersection(
                            (cameraPosition + possibleSolution[0]*rayDirection), 
                            possibleSolution[0],
                            triangles[i],
                            i));
        } else if (((possibleSolution[1] >= 0.0) && (possibleSolution[1]  <= 1.0)) &&
            ((possibleSolution[2]  >= 0.0) && (possibleSolution[2] <= 1.0)) &&
            ((possibleSolution[1]  + possibleSolution[2]) <= 1.0) && 
            (possibleSolution[0] < output[output.size()-1].distanceFromCamera) &&
            (possibleSolution[0] >= 0)){
                output.push_back(RayTriangleIntersection(
                            (cameraPosition + possibleSolution[0]*rayDirection), 
                            possibleSolution[0],
                            triangles[i],
                            i));
        }
    }

    if (output.size() == 0) {
        return RayTriangleIntersection(glm::vec3(0), -1, ModelTriangle(glm::vec3(0), glm::vec3(0), glm::vec3(0), Colour(0,0,0)), -1);
    } else {
        return output[output.size()-1];
    }
}

void RayTracedRenderer::draw(DrawingWindow &window) {
    window.clearPixels();
    // int xCameraOffset = round(window.width/2);
    // int yCameraOffset = round(window.height/2);
    // std::vector<std::vector<float>> zDepths (window.width, std::vector<float>(window.height,0));

    // if (triangles.size() > 0) {
    //     for (int i = 0; i < triangles.size();i++) {
    //         CanvasTriangle targetTriangle;
    //         for (int j = 0; j <= 2; j++) {
    //             CanvasPoint projectedVertex = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangles[i].vertices[j] * cameraOrientation);
    //             glm::vec3 cameraVertex(projectedVertex.x*imagePlaneScaling+xCameraOffset, window.height-(projectedVertex.y*imagePlaneScaling+yCameraOffset), projectedVertex.depth);
    //             glm::vec3 targetVertex(cameraVertex);
    //             targetTriangle[j] = CanvasPoint(targetVertex.x, targetVertex.y, targetVertex.z);
    //         }
    //         drawFilledTriangle(window, targetTriangle, triangles[i].colour, false, zDepths);
    //         // drawStrokedTriangle(window, targetTriangle, triangles[i].colour);
    //     }
    // }
}
