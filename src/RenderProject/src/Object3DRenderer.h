#pragma once
#include <glm/glm.hpp>
#include "RasterisedRenderer.h"
#include <CanvasTriangle.h>
#include <TextureMap.h>
#include <map>

//remember, our backwards is forwards and that means a +z.
enum direction {LEFT, RIGHT, UP, DOWN, FORWARDS, BACKWARDS, RXP, RYP, RXN, RYN, PLEFT, PRIGHT, PUP, PDOWN};
enum rendererType {WIREFRAME, RASTERISED, RAYTRACED};


class Object3DRenderer: public RasterisedRenderer {
   protected:
      std::vector<ModelTriangle> triangles;
      std::vector<Colour> palette;
      std::map<std::string, TextureMap> textures;
      glm::vec3 cameraPosition = glm::vec3(0.0, 0.0, 4.0);
      float imagePlaneScaling = 160; 
      float focalLength = 2.0;
      glm::mat3 cameraOrientation = glm::mat3({1,0,0}, {0,1,0}, {0,0,1});
      glm::vec3 lightPosition = glm::vec3(0.0, 0.8, 0.0);
      glm::vec3 vertical = glm::vec3(0.0 ,1.0 ,0.0);
      glm::vec3 lookAtCentre = glm::vec3(0.0, 0.0, 0.0);
      bool lookAt = false;
   public:
      void draw(DrawingWindow &window) override;
      void drawRasterisedScene(DrawingWindow &window, rendererType renderType);
      void readOBJfile(std::string objPath, float scale);
      void translateCamera(direction d);
      void rotateCamera(direction d);
      void setLookAt(bool value, glm::vec3 targetCentre);
      bool getLookAt();
   private:
      Colour getColour(std::ifstream &mtlFile, std::string targetColour, std::string objPath);
      CanvasPoint projectVertexOntoCanvasPoint(glm::vec3 cameraPosition, float focalLength, glm::vec3 vertexPosition);
      void setOrientationCentre();
};
