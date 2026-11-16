#include "RasterisedRenderer.h"
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <CanvasTriangle.h>
#include <algorithm>

float pythagoras(CanvasPoint a, CanvasPoint b){
    float xDiff = a.x - b.x;
    float yDiff = a.y - b.y;
    return sqrt(xDiff*xDiff + yDiff*yDiff);
}



std::vector<CanvasPoint> interpolateCanvasPointValues(CanvasPoint from, CanvasPoint to) {
   std::vector<CanvasPoint> result;
   
   float xDiff = to.x - from.x;float yDiff = to.y - from.y; float zDiff = to.depth - from.depth;
   float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
   float xStepSize = xDiff/numberOfSteps;
   float yStepSize = yDiff/numberOfSteps;
   float zStepSize = zDiff/numberOfSteps;
   if (numberOfSteps == 0) return {from};

   for (float i = 0.0; i<=numberOfSteps; i++) {
      int x = round(from.x + (xStepSize*i));
      int y = round(from.y + (yStepSize*i));
      float z = from.depth + (zStepSize*i);
      result.push_back(CanvasPoint(x,y,z));
   }

   return result;
}

//rewrote the interpolation code for pairs cus the canvaspoint one is kinda fucked
std::vector<TexturePoint> interpolateTexturePointValues(TexturePoint from, TexturePoint to, int numberOfValues) {
   std::vector<TexturePoint> result;
   float xStep = (to.x - from.x)/(numberOfValues);
   float yStep = (to.y - from.y)/(numberOfValues);
   for (size_t i = 0; (i <= numberOfValues); i++) {
      result.push_back(TexturePoint(round(from.x+(i*xStep)), round(from.y+(i*yStep))));
   };
   return result;
}

uint32_t grabTexturePixelValue (TexturePoint tPoint, const TextureMap &tMap) {
   // int position = (tMap.width*tPoint.y) + tPoint.x;
   // uint32_t pixel = tMap.pixels[position];

   // return Colour(
   //    (pixel >> 16) & 0xFF,
   //    (pixel >> 8)  & 0xFF, 
   //    pixel & 0xFF);
   return tMap.pixels[tMap.width*tPoint.y + tPoint.x];
}

void plotPixel(DrawingWindow &window, int x, int y, uint32_t inputColour) {
   // uint32_t colour = (255 << 24) + (inputColour.red << 16) + (inputColour.green << 8) + inputColour.blue;  
   if ((x < window.width) && (x >= 0) && (y < window.height) && (y >= 0)) window.setPixelColour(x, y, inputColour);
}

//based on row interpolation
void interpolateAndDrawTexturePointValues(DrawingWindow &window, CanvasPoint cFrom, CanvasPoint cTo, TexturePoint tFrom, TexturePoint tTo, TextureMap& tMap) {
   float numberOfValues = std::abs(cTo.x - cFrom.x);

   if (numberOfValues == 0) {
      plotPixel(window, cFrom.x, cFrom.y, grabTexturePixelValue(tFrom, tMap));
      return;
   }
   
   float xStepT = (tTo.x - tFrom.x)/(numberOfValues);
   float yStepT = (tTo.y - tFrom.y)/(numberOfValues);
   float xStepAcc = 0;
   float yStepAcc = 0; 
   int polarity = 1;
   if (xStepT < 0) polarity = -1;
   for (size_t i = 0; (i <= numberOfValues); i++) {
      plotPixel(window, cFrom.x + (polarity * i), cFrom.y, grabTexturePixelValue(TexturePoint(round(tFrom.x + xStepAcc), round(tFrom.y + yStepAcc)), tMap));
      xStepAcc += xStepT;
      yStepAcc += yStepT;
   };

}

bool sortTriangleByY (CanvasPoint& a, CanvasPoint& b) {
    return (a.y < b.y);
}

CanvasTriangle sortTriangle (CanvasTriangle triangle) {
   std::vector<CanvasPoint> triangleVector; 
   triangleVector.push_back(triangle[0]);
   triangleVector.push_back(triangle[1]);
   triangleVector.push_back(triangle[2]);
   std::sort(triangleVector.begin(), triangleVector.end(), sortTriangleByY);
   triangle[0] = triangleVector[0];
   triangle[1] = triangleVector[1];
   triangle[2] = triangleVector[2];
   return triangle;
}


void drawLine(DrawingWindow &window, std::vector<CanvasPoint> interpolatedValues, Colour inputColour) {
   for (int i = 0; i < interpolatedValues.size(); i++) {
      CanvasPoint currentPixelPoint = interpolatedValues[i];
      uint32_t colour = (255 << 24) + (inputColour.red << 16) + (inputColour.green << 8) + inputColour.blue;  
      plotPixel(window, currentPixelPoint.x, currentPixelPoint.y, colour);
   }
}

void drawLine(DrawingWindow &window, std::vector<CanvasPoint> interpolatedValues, Colour inputColour, std::vector<std::vector<float>> &zDepths) {
   for (int i = 0; i < interpolatedValues.size(); i++) {
      CanvasPoint currentPixelPoint = interpolatedValues[i];
      if ((currentPixelPoint.x < window.width) && (currentPixelPoint.y < window.height) && (currentPixelPoint.x >= 0) && (currentPixelPoint.y >= 0)) {
         if (currentPixelPoint.depth > zDepths[currentPixelPoint.x][currentPixelPoint.y]) {
            zDepths[currentPixelPoint.x][currentPixelPoint.y] = currentPixelPoint.depth;
            uint32_t colour = (255 << 24) + (inputColour.red << 16) + (inputColour.green << 8) + inputColour.blue;  
            plotPixel(window, currentPixelPoint.x, currentPixelPoint.y, colour);
         }
      }
   }
}

//redundant
void drawTextureLine(DrawingWindow &window, std::vector<CanvasPoint> interpolatedTValues, std::vector<TexturePoint> interpolatedMValues, TextureMap &texture) {
   for (int i = 0; i < interpolatedTValues.size(); i++) {
      plotPixel(window, interpolatedTValues[i].x, interpolatedTValues[i].y, grabTexturePixelValue(interpolatedMValues[i], texture));
   }
}

void RasterisedRenderer::draw(DrawingWindow &window) {
   // window.clearPixels();
   // Colour white (255,255,255);

   // //top left to middle
   // CanvasPoint topLeft (0,0);
   // CanvasPoint middle (window.width/2, window.height/2);
   // std::vector<CanvasPoint> leftLine = interpolateCanvasPointValues(topLeft, middle);
   // drawLine(window, leftLine, white);

   // //top right to middle
   // CanvasPoint topRight (window.width, 0);
   // std::vector<CanvasPoint> rightLine = interpolateCanvasPointValues(topRight, middle);
   // drawLine(window, rightLine, white);

   // //vertical down the middle
   // CanvasPoint topMiddle (window.width/2, 0);
   // CanvasPoint bottomMiddle (window.width/2,window.height);
   // std::vector<CanvasPoint> middleLine = interpolateCanvasPointValues(topMiddle, bottomMiddle);
   // drawLine(window, middleLine, white);

   // //horizontal line third the width of the screen, centred
   // CanvasPoint shortLeft (window.width/3, window.height/2);
   // CanvasPoint shortRight (window.width*2/3, window.height/2);
   // std::vector<CanvasPoint> shortLine = interpolateCanvasPointValues(shortLeft, shortRight);
   // drawLine(window, shortLine, white);
}

void RasterisedRenderer::drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
   std::vector<CanvasPoint> line1 = interpolateCanvasPointValues(triangle.v0(), triangle.v1());
   std::vector<CanvasPoint> line2 = interpolateCanvasPointValues(triangle.v0(), triangle.v2());
   std::vector<CanvasPoint> line3 = interpolateCanvasPointValues(triangle.v1(), triangle.v2());
   drawLine(window, line1, colour);
   drawLine(window, line2, colour);
   drawLine(window, line3, colour);
}

void RasterisedRenderer::drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour, bool stroke) {
   
   triangle = sortTriangle(triangle);

   std::vector<CanvasPoint> topLine = interpolateCanvasPointValues(triangle[0], triangle[1]);
   std::vector<CanvasPoint> bottomLine = interpolateCanvasPointValues(triangle[1], triangle[2]);
   float longLineYtoXRatio = (triangle[2].x - triangle[0].x) / (triangle[2].y - triangle[0].y);

   for (int i = 0; i < topLine.size(); i++) {
      int longX = round(triangle[0].x + longLineYtoXRatio * (topLine[i].y - triangle[0].y));
      std::vector<CanvasPoint> raster = interpolateCanvasPointValues(topLine[i], CanvasPoint(longX, topLine[i].y));
      drawLine(window, raster, colour);
   }

   for (int i = 0; i < bottomLine.size(); i++) {
      int longX = round(triangle[0].x + longLineYtoXRatio * (bottomLine[i].y - triangle[0].y));
      std::vector<CanvasPoint> raster = interpolateCanvasPointValues(bottomLine[i], CanvasPoint(longX, bottomLine[i].y));
      drawLine(window, raster, colour);
   }

   if (stroke) drawStrokedTriangle(window, triangle, Colour(255,255,255));
}


void RasterisedRenderer::drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour, bool stroke, std::vector<std::vector<float>> &zDepths) {
   
   triangle = sortTriangle(triangle);


   std::vector<CanvasPoint> topLine = interpolateCanvasPointValues(triangle[0], triangle[1]);
   std::vector<CanvasPoint> bottomLine = interpolateCanvasPointValues(triangle[1], triangle[2]);
   
   float dy = triangle[2].y - triangle[0].y;
   if (std::abs(dy) < 1.0f) {
      return;
   }
   float longLineYtoXRatio = (triangle[2].x - triangle[0].x) / dy;
   //float longLineYtoXRatio = (triangle[2].x - triangle[0].x) / (triangle[2].y - triangle[0].y);
   float longLineYtoZRatio = (triangle[2].depth - triangle[0].depth) / (triangle[2].y - triangle[0].y);

   for (int i = 0; i < topLine.size(); i++) {
      int longX = round(triangle[0].x + longLineYtoXRatio * (topLine[i].y - triangle[0].y));
      float longZ = triangle[0].depth + longLineYtoZRatio * (topLine[i].y - triangle[0].y);
      std::vector<CanvasPoint> raster = interpolateCanvasPointValues(topLine[i], CanvasPoint(longX, topLine[i].y, longZ));
      drawLine(window, raster, colour, zDepths);
   }

   for (int i = 0; i < bottomLine.size(); i++) {
      int longX = round(triangle[0].x + longLineYtoXRatio * (bottomLine[i].y - triangle[0].y));
      float longZ = triangle[0].depth + longLineYtoZRatio * (bottomLine[i].y - triangle[0].y);
      std::vector<CanvasPoint> raster = interpolateCanvasPointValues(bottomLine[i], CanvasPoint(longX, bottomLine[i].y, longZ));
      drawLine(window, raster, colour, zDepths);
   }

   if (stroke) drawStrokedTriangle(window, triangle, Colour(255,255,255));
}

void RasterisedRenderer::drawTextureTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap &texture) {
   //FIXME: doesn't have zdepth logic

   triangle = sortTriangle(triangle);

   std::vector<CanvasPoint> topLine = interpolateCanvasPointValues(triangle[0], triangle[1]);
   std::vector<TexturePoint> topTextureLine = interpolateTexturePointValues(triangle[0].texturePoint, triangle[1].texturePoint, topLine.size());


   std::vector<CanvasPoint> bottomLine = interpolateCanvasPointValues(triangle[1], triangle[2]);
   std::vector<TexturePoint> bottomTextureLine = interpolateTexturePointValues(triangle[1].texturePoint, triangle[2].texturePoint, bottomLine.size());

   float longLineYtoXRatio = (triangle[2].x - triangle[0].x) / (triangle[2].y - triangle[0].y);
   float longXDiff = triangle[2].x - triangle[0].x;
   float longYDiff = triangle[2].y - triangle[0].y;
   float textureXDiff = triangle[2].texturePoint.x - triangle[0].texturePoint.x;
   float textureYDiff = triangle[2].texturePoint.y - triangle[0].texturePoint.y;

   int longX; int longTextureX; int longTextureY;
   float xRatio = textureXDiff / longXDiff;
   float yRatio = textureYDiff / longYDiff;


   for (int i = 0; i < topLine.size(); i++) {
      longX = round(triangle[0].x + longLineYtoXRatio * (topLine[i].y - triangle[0].y));
      longTextureX = round(triangle[0].texturePoint.x + (longX - triangle[0].x) * xRatio);
      longTextureY = round(triangle[0].texturePoint.y + (topLine[i].y - triangle[0].y) * yRatio);
      interpolateAndDrawTexturePointValues(window, topLine[i], CanvasPoint(longX, topLine[i].y), topTextureLine[i], TexturePoint(longTextureX, longTextureY), texture);
   }

   for (int i = 0; i < bottomLine.size(); i++) {
      longX = round(triangle[0].x + longLineYtoXRatio * (bottomLine[i].y - triangle[0].y));
      longTextureX = round(triangle[0].texturePoint.x + (longX - triangle[0].x) * xRatio);
      longTextureY = round(triangle[0].texturePoint.y + (bottomLine[i].y - triangle[0].y) * yRatio);
      interpolateAndDrawTexturePointValues(window, bottomLine[i], CanvasPoint(longX, bottomLine[i].y), bottomTextureLine[i], TexturePoint(longTextureX, longTextureY), texture);
   }

   drawStrokedTriangle(window, triangle, Colour(255,255,255));
}