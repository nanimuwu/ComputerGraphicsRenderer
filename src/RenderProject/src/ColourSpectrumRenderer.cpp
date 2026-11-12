#include "ColourSpectrumRenderer.h"
#include <glm/glm.hpp>



std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
   std::vector<float> result;
   float step = (to - from)/(numberOfValues-1);
   for (size_t i = 0; (i < numberOfValues); i++) {
      result.push_back(from+(i*step));
   };
   return result;
}


std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, float numberOfValues) {
   std::vector<glm::vec3> result;
   glm::vec3 step = (to - from) / (numberOfValues-1.0f);

   for (float i = 0.0; (i < numberOfValues); i++) {
      result.push_back(from+(step*i));
   };
   return result;
}

void ColourSpectrumRenderer::draw(DrawingWindow &window) {
   window.clearPixels();

   // Greyscale -> white on left, black on right

   // std::vector<float> greyscaleVector = interpolateSingleFloats(255, 0, window.width);
   // for (size_t y = 0; y < window.height; y++) {
	// 	for (size_t x = 0; x < window.width; x++) {
	// 		float red = greyscaleVector[x];
	// 		float green = greyscaleVector[x];
	// 		float blue = greyscaleVector[x];
	// 		uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
	// 		window.setPixelColour(x, y, colour);
	// 	}
	// }


   // Colour rectangle -> red, blue, yellow, green

   // glm::vec3 topLeft(255, 0, 0);        // red 
   // glm::vec3 topRight(0, 0, 255);       // blue 
   // glm::vec3 bottomRight(0, 255, 0);    // green 
   // glm::vec3 bottomLeft(255, 255, 0);   // yellow
   
   // std::vector<glm::vec3> leftColumnInterpolation = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
   // std::vector<glm::vec3> rightColumnInterpolation = interpolateThreeElementValues(topRight, bottomRight, window.height);
   
   // for (size_t y = 0; y < window.height; y++) {
   //    std::vector<glm::vec3> rowInterpolation = interpolateThreeElementValues(leftColumnInterpolation[y], rightColumnInterpolation[y], window.width);
	// 	for (size_t x = 0; x < window.width; x++) {
	// 		float red = rowInterpolation[x][0];
	// 		float green = rowInterpolation[x][1];
	// 		float blue = rowInterpolation[x][2];
	// 		uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
	// 		window.setPixelColour(x, y, colour);
	// 	}
	// }
   

   //Triangle -> red, green, blue

   glm::vec2 v0(0, window.height); //red
   glm::vec2 v1(window.width/2.0f, 0.0f); //green  
   glm::vec2 v2(window.width, window.height); //blue

   float red = 0.0f;
   float green = 0.0f;
   float blue = 0.0f;

   for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
         
         glm::vec3 barycentricPixelValue = convertToBarycentricCoordinates(v0,v1,v2,glm::vec2(x,y));
         if ((barycentricPixelValue[0] >= 0) and (barycentricPixelValue[1] >= 0) and (barycentricPixelValue[2] >= 0)) {
            red = 255.0f * barycentricPixelValue[2]; //w
            green = 255.0f * barycentricPixelValue[0]; //u
            blue = 255.0f * barycentricPixelValue[1]; //v
         } else {
               red = 0.0f;
               green = 0.0f;
               blue = 0.0f;
         }
         uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);  
			window.setPixelColour(x, y, colour);
		}
	}
}