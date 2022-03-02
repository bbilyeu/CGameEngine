#ifndef GLM_UTIL_H
#define GLM_UTIL_H

#include <glm/glm.hpp>

/// glm functions
glm::vec4 convertV4ScrToGLScissor(const glm::vec4& coords, float scrWidth, float scrHeight);
glm::vec2 convertV2ScrToNDC(const glm::vec2& coords, float scrWidth, float scrHeight); // convert to NDC (normalized device coordinates)
glm::vec4 convertV4ScrToNDC(const glm::vec4& coords, float scrWidth, float scrHeight); // convert to NDC (normalized device coordinates)
glm::vec2 convertV2ScrToWorld(const glm::vec2& coords, float scrWidth, float scrHeight); // convert to world coordinates
glm::vec4 convertV4ScrToWorld(const glm::vec4& coords, float scrWidth, float scrHeight); // convert to world coordinates
glm::vec2 getSlope(glm::vec4 pos, float angle);
glm::vec2 rotatePoint(glm::vec2 pos, float angle);
glm::vec3 rotatePointXY(glm::vec3 pos, float angle);


#endif // GLM_UTIL_H
