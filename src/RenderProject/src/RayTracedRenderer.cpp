#include "RayTracedRenderer.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

const float pi = 3.1415926535897932384626;

enum rendererType currentRendererType = RAYTRACED;
const float lightScalar = 10.0f;
const float specularPower = 64;

bool gouraud = false;
bool phong = false;
struct vertexColours {
    std::array<Colour,3> colours;
    bool calculated = false;
};

std::vector<vertexColours> triangleVertexColours;

void RayTracedRenderer::toggleGouraud(){
    phong = false;
    gouraud = !gouraud; 
}

void RayTracedRenderer::togglePhong(){
    gouraud = false;
    phong = !phong; 
}

void RayTracedRenderer::setRenderType(rendererType renderType) {
    currentRendererType = renderType;
}

Colour getTexturePixelColour(TexturePoint t0, TexturePoint t1, TexturePoint t2, glm::vec3 barycentricCoordinate, TextureMap &tMap) {
    float u = barycentricCoordinate[1];
    float v = barycentricCoordinate[2];
    float w = 1-u-v;
    float targetX = t0.x*w + t1.x*u + t2.x*v;
    float targetY = t0.y*w + t1.y*u + t2.x*v;
    uint32_t rawColour = tMap.pixels[tMap.width*targetY + targetX];
    Colour colour = Colour(
        (rawColour>>16) & 0xFF,
        (rawColour>>8) & 0xFF,
        rawColour & 0xFF
    );
    return colour;
}

IntersectionResult RayTracedRenderer::getClosestIntersection(glm::vec3 startingPoint, glm::vec3 rayDirection){
    
    IntersectionResult output; output.targetTriangle.distanceFromCamera = std::numeric_limits<float>::infinity();
    bool found = false;

    for (int i = 0; i<triangles.size(); i++) {
        glm::vec3 e0 = triangles[i].vertices[1] - triangles[i].vertices[0];
        glm::vec3 e1 = triangles[i].vertices[2] - triangles[i].vertices[0];
        glm::vec3 SPVector = startingPoint - triangles[i].vertices[0];
        glm::mat3 DEMatrix(-rayDirection, e0, e1);
        glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector; //(t,u,v) -> t is distance, u & v are barycentric coords
        
        if (((possibleSolution[1] >= 0.0) && (possibleSolution[1]  <= 1.0)) &&
            ((possibleSolution[2]  >= 0.0) && (possibleSolution[2] <= 1.0)) &&
            ((possibleSolution[1]  + possibleSolution[2]) <= 1.0) && 
            (possibleSolution[0] < output.targetTriangle.distanceFromCamera) &&
            (possibleSolution[0] > 0.001f)){
                output.targetTriangle = RayTriangleIntersection(
                            (startingPoint + possibleSolution[0]*rayDirection), 
                            possibleSolution[0],
                            triangles[i],
                            i);
                output.barycentric = possibleSolution;
                found = true;
        }
    }

    if (!found) {
        output.targetTriangle = RayTriangleIntersection(glm::vec3(0), -1, ModelTriangle(glm::vec3(0), glm::vec3(0), glm::vec3(0), Colour(0,0,0)), -1);
        output.barycentric = glm::vec3(-1,-1,-1);
    }
    return output;
}

Colour updateColourIntensity(Colour initialColour, float distanceToLight, glm::vec3 cameraDirection, glm::vec3 lightDirection, glm::vec3 normalVector) {
    float distMult = 1.0f/(pi*distanceToLight*distanceToLight);
    float aoi = std::max(0.0f, glm::dot(lightDirection, normalVector));
    float specularSpread = 0.0f;

    if (aoi > 0.0f) {
        glm::vec3 outgoingLight = lightDirection - 2.0f*normalVector*(glm::dot(lightDirection, normalVector));
        specularSpread = std::pow(std::max(glm::dot(cameraDirection, outgoingLight), 0.0f), specularPower);
    }

    float mult = std::min(1.0f, lightScalar*distMult*aoi);
    float newRed = std::min(255.0f, mult*initialColour.red+(255*specularSpread*mult));
    float newGreen = std::min(255.0f, mult*initialColour.green+(255*specularSpread*mult));
    float newBlue = std::min(255.0f, mult*initialColour.blue+(255*specularSpread*mult));
    return Colour(newRed, newGreen, newBlue);
}

void RayTracedRenderer::updateTriangleVertexInfo(RayTriangleIntersection &targetTriangle) {
    triangleVertexColours[targetTriangle.triangleIndex].calculated = true;

    for (int i = 0; i < 3; i++) {
        glm::vec3 rayDirection = glm::normalize(targetTriangle.intersectedTriangle.vertices[i] - cameraPosition);
        glm::vec3 pointToLight = lightPosition - targetTriangle.intersectedTriangle.vertices[i];
        triangleVertexColours[targetTriangle.triangleIndex].colours[i] = updateColourIntensity(targetTriangle.intersectedTriangle.colour, glm::length(pointToLight), glm::normalize(rayDirection), glm::normalize(pointToLight), vertexNormals[targetTriangle.triangleIndex][i]);
    }
}

Colour getGourardColour(std::array<Colour, 3> colours, glm::vec3 barycentricCoordinates) {
    float u = barycentricCoordinates[1];
    float v = barycentricCoordinates[2];
    float w = 1-u-v;
    float r = colours[0].red * w + colours[1].red * u + colours[2].red * v;
    float g = colours[0].green * w + colours[1].green * u + colours[2].green * v;
    float b = colours[0].blue * w + colours[1].blue * u + colours[2].blue * v;
    return Colour(r,g,b);
}

glm::vec3 getPhongNormal(glm::mat3 &vertexNormals, glm::vec3 barycentricCoordinates) {
    float u = barycentricCoordinates[1];
    float v = barycentricCoordinates[2];
    float w = 1-u-v;
    return (glm::normalize(w*vertexNormals[0] + u *vertexNormals[1] + v *vertexNormals[2]));
}

void RayTracedRenderer::draw(DrawingWindow &window) {
    window.clearPixels();

    switch (currentRendererType) {
        case (RAYTRACED):
            if (gouraud) triangleVertexColours.assign(triangles.size(), vertexColours());
            for (int y = 0; y < window.height; y++) {
                for (int x = 0; x < window.width; x++) {
                    float u = (x - window.width  / 2.0f) / imagePlaneScaling;
                    float v = (window.height / 2.0f - y) / imagePlaneScaling;

                    glm::vec3 camDir(u, v, -focalLength);
                    glm::vec3 rayDirection = glm::normalize(cameraOrientation * camDir);

                    IntersectionResult intersection = getClosestIntersection(cameraPosition, rayDirection);
                    RayTriangleIntersection targetTriangle = intersection.targetTriangle;
                    glm::vec3 targetTriangleBarycentricCoordinates = intersection.barycentric;
                    
                    //if hit triangle
                    if (targetTriangle.triangleIndex != -1) {
                        //TODO: add texture checking here

                        std::array<TexturePoint, 3> &tP = targetTriangle.intersectedTriangle.texturePoints;
                        const TexturePoint originTexturePoint(0,0);
                        if (tP[0].x != originTexturePoint.x || tP[0].y != originTexturePoint.y ||
                            tP[1].x != originTexturePoint.x || tP[1].y != originTexturePoint.y ||
                            tP[2].x != originTexturePoint.x || tP[2].y != originTexturePoint.y) {
                                auto targetTexture = textures.find(targetTriangle.intersectedTriangle.colour.name);
                                if (targetTexture != textures.end()){
                                    TextureMap &map = targetTexture->second;
                                    targetTriangle.intersectedTriangle.colour = getTexturePixelColour(tP[0], tP[1], tP[2], targetTriangleBarycentricCoordinates, map); 
                            }
                        }
                        glm::vec3 pointToLight = lightPosition - targetTriangle.intersectionPoint;
                        IntersectionResult lightIntersectionResult = getClosestIntersection(targetTriangle.intersectionPoint, glm::normalize(pointToLight));
                        RayTriangleIntersection lightIntersection = lightIntersectionResult.targetTriangle;
                        Colour minColour = Colour(
                                    0.1 * targetTriangle.intersectedTriangle.colour.red,
                                    0.1 * targetTriangle.intersectedTriangle.colour.green,
                                    0.1 * targetTriangle.intersectedTriangle.colour.blue);
                        Colour c;

                        if (gouraud) {
                            if (!triangleVertexColours[targetTriangle.triangleIndex].calculated){
                                updateTriangleVertexInfo(targetTriangle);
                            } 

                            c = getGourardColour(triangleVertexColours[targetTriangle.triangleIndex].colours, targetTriangleBarycentricCoordinates);
                        
                        } else if (phong) {
                            glm::vec3 interpolatedNormal = getPhongNormal(vertexNormals[targetTriangle.triangleIndex], targetTriangleBarycentricCoordinates);
                            c = updateColourIntensity(targetTriangle.intersectedTriangle.colour, glm::length(pointToLight), glm::normalize(rayDirection), glm::normalize(pointToLight), interpolatedNormal); 
                        
                        } else {
                            c = updateColourIntensity(targetTriangle.intersectedTriangle.colour, glm::length(pointToLight), glm::normalize(rayDirection), glm::normalize(pointToLight), targetTriangle.intersectedTriangle.normal);
                        }

                        //if in shadow
                        if ((lightIntersection.triangleIndex != -1) && (lightIntersection.distanceFromCamera < glm::length(pointToLight))) {
                            c = minColour;
                        }
                       
                        //if too far away from the light source 
                        if (glm::length(glm::vec3(c.red, c.green, c.blue)) < glm::length(glm::vec3(minColour.red, minColour.green, minColour.blue))) {
                            c = minColour;
                        }
                        

                        uint32_t colour = (255 << 24) + (c.red << 16) + (c.green << 8) + c.blue;
                        window.setPixelColour(x, y, colour);
                    }
                }
            }
            break;

        case (WIREFRAME):
            drawRasterisedScene(window, currentRendererType);
            break;
        case (RASTERISED):
            drawRasterisedScene(window, currentRendererType);
            break;
    }
}

