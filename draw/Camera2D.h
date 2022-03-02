#ifndef CAMERA2D_H
#define CAMERA2D_H
#include <glm/glm.hpp>


/// \TODO: Create method for storing/updating min/map clip plane, set in the init() function, change elsewhere

namespace CGameEngine
{
    class Camera2D
    {
        public:
            Camera2D();
            ~Camera2D();
            bool isBoxInView(const glm::vec2& position, const glm::vec2& dimensions); // camera culling or viewplane
            void init(int scrWidth, int scrHeight);
            void update();
            // sets
            void setMininumScale(float val) { m_scaleRange[0] = val; }
            void setMaximumScale(float val) { m_scaleRange[1] = val; }
            void setPosition(const glm::vec3& newPosition) { m_position = newPosition; m_hasChanged = true; }
            void setPosition(const glm::vec2& newPosition) { m_position = glm::vec3(newPosition.x, newPosition.y, 0.0f); m_hasChanged = true; }
            void setScale(float newScale);
            void setScaleRange(glm::vec2 val) { m_scaleRange = val; }
            void setScaleRange(float minScale, float maxScale) { m_scaleRange = glm::vec2(minScale, maxScale); }

            // zoom values are inverted as we're moving the camera AWAY from the target
            void zoomIn(float val = 1.0f) { tryZoom(-val); }
            void zoomOut(float val = 1.0f) { tryZoom(val); }

            // gets
            const glm::vec3& getPosition() const { return m_position; }
            const float& getScale() const { return m_scale; }
            const glm::mat4& getCameraMatrix() const { return m_cameraMatrix; }
            glm::vec3 convertScreenToWorld(glm::vec3 screenCoords);
            glm::mat4 getOrtho();

        private:
            glm::vec2 m_scaleRange = glm::vec2(1.0f, 128.0f);
            glm::vec3 m_position = glm::vec3(0.0f);
            glm::mat4 m_cameraMatrix = glm::mat4(0.0f); // "view" in "M-V-P" setup
            glm::mat4 m_projectionMatrix;
            bool m_hasChanged = true;
            float m_scale = 1.0f;
            float m_fieldOfView = 45.0f;
            int m_screenWidth = 1280, m_screenHeight = 960;

            void tryZoom(float val);

            const glm::vec3 CAMERA_FRONT = glm::vec3(0.0f, 0.0f, -1.0f);
            const glm::vec3 CAMERA_RIGHT = glm::vec3(0.0f, 1.0f, 0.0f);
    };
}
#endif // CAMERA2D_H
