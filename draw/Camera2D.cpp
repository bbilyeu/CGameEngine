#include "draw/Camera2D.h"
#include "common/types.h"
#include <glm/gtc/matrix_transform.hpp> // orthographic view

namespace CGameEngine
{
    Camera2D::Camera2D() {} // empty

    Camera2D::~Camera2D() {} // empty

    /**
     * setup the camera object
     *
     * @param scrWidth screen width in pixels
     * @param scrHeight screen height in pixels
     */
    void Camera2D::init(int scrWidth, int scrHeight)
    {
		m_screenWidth = scrWidth;
		m_screenHeight = scrHeight;
		m_projectionMatrix = glm::perspective(glm::radians(45.0f), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);
	}

    /**
     * Simple AABB test to see if a box is in the camera view
     *
     * @param position x/y to test against
     * @param dimensions z/w (size) of the object
     */
    bool Camera2D::isBoxInView(const glm::vec2& position, const glm::vec2& dimensions)
    {
        // account for scaling (zooming)
        glm::vec2 scaledScreenDimensions = glm::vec2(m_screenWidth, m_screenHeight) / m_scale;
        // The minimum distance before a collision occurs
        const float MIN_DISTANCE_X = dimensions.x / 2.0f + scaledScreenDimensions.x / 2.0f;
        const float MIN_DISTANCE_Y = dimensions.y / 2.0f + scaledScreenDimensions.y / 2.0f;

        // Center position of the entity
        glm::vec2 centerPos = position + dimensions / 2.0f;
        // Center position of the camera
        glm::vec2 centerCameraPos = glm::vec2(m_position.x, m_position.y);
        // get input from the entity to compare to camera
        glm::vec2 distVec = centerPos - centerCameraPos;

        // Get the depth of the collision
        float xDepth = MIN_DISTANCE_X - abs(distVec.x);
        float yDepth = MIN_DISTANCE_Y - abs(distVec.y);

        // If either the depths are > 0, then we collided
        if (xDepth > 0 && yDepth > 0) {
            return true;
        }

        // else
        return false;
    }

    /**
    * create projection view for M,V,P calculation
    */
    void Camera2D::update()
    {
        if(m_hasChanged)
        {
            m_hasChanged = false;
            m_cameraMatrix = glm::lookAt(m_position, CAMERA_FRONT, CAMERA_RIGHT);
            m_cameraMatrix = m_projectionMatrix * m_cameraMatrix; // projection * view

			Logger::getInstance().Log(Logs::DEBUG, "Camera2D::update()",
				"\n(\t{},\t{},\t{},\t{}\t)\n(\t{},\t{},\t{},\t{}\t)\n(\t{},\t{},\t{},\t{}\t)\n(\t{},\t{},\t{},\t{}\t)\n",
				m_cameraMatrix[0][0], m_cameraMatrix[0][1], m_cameraMatrix[0][2], m_cameraMatrix[0][3],
				m_cameraMatrix[1][0], m_cameraMatrix[1][1], m_cameraMatrix[1][2], m_cameraMatrix[1][3],
				m_cameraMatrix[2][0], m_cameraMatrix[2][1], m_cameraMatrix[2][2], m_cameraMatrix[2][3],
				m_cameraMatrix[3][0], m_cameraMatrix[3][1], m_cameraMatrix[3][2], m_cameraMatrix[3][3] );
			Logger::getInstance().Log(Logs::DEBUG, "Camera2D::update()", "Position ({}, {}, {})", m_position.x, m_position.y, m_position.z);
        }
    }

    /**
     * set the zoom (scale) of the camera
     *
     * @param newScale zoom level (1.0 being default)
     */
    void Camera2D::setScale(float newScale)
    {
        m_scale = newScale;
        m_position.z = m_scale;
        m_hasChanged = true;
    }

    /**
     * convert passed screen coordinates into world coordinates
     *
     * @param screenCoords x/y/z coordinates
     */
    glm::vec3 Camera2D::convertScreenToWorld(glm::vec3 screenCoords)
    {
        screenCoords.y = m_screenHeight - screenCoords.y; // invert y
        screenCoords -= glm::vec3(m_screenWidth / 2, m_screenHeight / 2, 0.0f); // make 0 the 'center'
        screenCoords /= m_scale; // account for scaling (zoom)
        // translated with the camera position
        screenCoords += m_position;
        return screenCoords;
    }

    /**
     * create orthographic
     */
    glm::mat4 Camera2D::getOrtho()
    {
		glm::mat4 retVal = glm::ortho(0.0f, (float)m_screenWidth, 0.0f, (float)m_screenHeight, -1.0f, 1.0f);
		return retVal;
    }

    /// Camera2D private functions below //////////////////////////////////////

    /**
     * @brief attempt to zoom in/out, min/max values permitting
     *
     * @param val the amount and direction (+3 = zoom in, -3 = zoom out)
     */
    void Camera2D::tryZoom(float val)
    {
        float newVal = m_position.z + val;
        if(newVal <= m_scaleRange[0]) { m_position.z = m_scaleRange[0]; }
        else if(newVal >= m_scaleRange[1]) { m_position.z = m_scaleRange[1]; }
        else { m_position.z = newVal; }
        m_hasChanged = true;
    }
}
