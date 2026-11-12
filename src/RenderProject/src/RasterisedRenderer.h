#pragma once

   #include "Renderer.h"
   #include <CanvasTriangle.h>
   #include <TextureMap.h>

class RasterisedRenderer: public Renderer {
   public:
      void draw(DrawingWindow &window) override;
      void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour);
      void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour, bool stroke);
      void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour, bool stroke, std::vector<std::vector<float>> &zDepths);
      void drawTextureTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap &texture);
};
