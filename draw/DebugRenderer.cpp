#include "draw/DebugRenderer.h"
//#include "phys/Body.h"
#include <glm/gtc/matrix_transform.hpp> // orthographic view
#include "draw/Shaders.h"

/// \TODO: Condense render() to 1 function with optional parameters
/// \TODO: Condense rotate point into 1 function or use the global one

const float PI = 3.14159265359f;

namespace CGameEngine
{

    DebugRenderer::DebugRenderer() : m_shaderProgram(), m_verts(), m_indices() {} // empty

    DebugRenderer::~DebugRenderer() { dispose(); }

    /**
     * @brief manual deconstructor in the event that the DebugRenderer is still needed
     */
    void DebugRenderer::dispose()
    {
        if(m_vao) { glDeleteVertexArrays(1, &m_vao); }
        if(m_vbo) { glDeleteBuffers(1, &m_vbo); }
        if(m_ibo) { glDeleteBuffers(1, &m_ibo); }

        m_shaderProgram.dispose();
        m_initialized = false;
        m_verts.clear();
        m_indices.clear();
        m_numElements = 0;
        m_vbo = 0;
        m_vao = 0;
		m_ibo = 0;
    }

    /*void DebugRenderer::drawBody(CGameEngine::Body* body, const glm::ivec4& color)
    {
        if(!body) { return; }

        switch(body->getBodyClass())
        {
			case BodyClass::boxBody:
			{
				glm::vec4 dr = body->getDestRect();
				drawBox(dr, color, body->getAngle());
				break;
			}
			case BodyClass::capsuleBody:
			{
				float angle = body->getAngle();
				glm::vec4 dr = body->getDestRect();
				drawBox(dr, color, angle); // center
				std::vector<std::vector<glm::vec2>> points = body->getPolygonPoints();
				points[1][0] = rotatePoint(points[1][0], angle);
				drawCircle(points[1][0], color, body->getDimensions().x * 0.5f);
				points[2][0] = rotatePoint(points[2][0], angle);
				drawCircle(points[2][0], color, body->getDimensions().x * 0.5f);
				break;

			}
			case BodyClass::polygonBody:
			{
                glm::vec2 center = body->getPosition();
                float angle = body->getAngle();
                std::vector<std::vector<glm::vec2>> points = body->getPolygonPoints();
                for(unsigned int i = 0; i < points.size(); i++)
                {
					for(unsigned int h = 0; h < points[i].size(); h++)
					{
						points[i][h] = rotatePoint(points[i][h], angle);
					}
                }
                drawCoordinates(points, color);
				break;
			}
			case BodyClass::sphereBody:
			{
				drawCircle(body->getPosition(), color, body->getDimensions().x * 0.5f);
				break;
			}
			default:
			{
				Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "DebugRenderer::drawBody()", "BodyClass is unknown! Max usable value is [{}], received [{}]", BodyClass::END-1, body->getBodyClass());
				return;
			}
        }

        // draw 'center' of each Body
        drawCircle(body->getPosition(), COLOR_BLACK, 0.05f);
    }*/

    /**
     * @brief draw a simple box
     *
     * @param destRect passed x/y/w/h
     * @param color RGBA color value (0-255)
     * @param angle angle in radians
     */
    void DebugRenderer::drawBox(const glm::vec4 destRect, const glm::ivec4& color, float angle)
    {
        int i = m_verts.size(); // location of the first vertex that we're modifying
        m_verts.resize(i + 4); // make room for the four new vertices
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "****************************");
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "point ({}, {})", destRect.x, destRect.y);
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "point ({}, {})", destRect.x + destRect.z, destRect.y);
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "point ({}, {})", destRect.x + destRect.z, destRect.y + destRect.w);
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "point ({}, {})", destRect.x, destRect.y + destRect.w);

        glm::vec3 halfDimensions(destRect.z / 2.0f, destRect.w / 2.0f, 0.0f);

        // get points centered at origin
        glm::vec3 tl(-halfDimensions.x, halfDimensions.y, 0.0f); // top left
        glm::vec3 bl(-halfDimensions.x, -halfDimensions.y, 0.0f); // bottom left
        glm::vec3 tr(halfDimensions.x, halfDimensions.y, 0.0f); // top right
        glm::vec3 br(halfDimensions.x, -halfDimensions.y, 0.0f); // bottom right

        glm::vec3 positionOffset(destRect.x, destRect.y, 0.0f); // offset of x,y

        // rotate the points and colorize
        m_verts[i].position = rotatePoint(bl, angle) + halfDimensions + positionOffset; m_verts[i].color = color;
        m_verts[i + 1].position = rotatePoint(br, angle) + halfDimensions + positionOffset; m_verts[i + 1].color = color;
        m_verts[i + 2].position = rotatePoint(tr, angle) + halfDimensions + positionOffset; m_verts[i + 2].color = color;
        m_verts[i + 3].position = rotatePoint(tl, angle) + halfDimensions + positionOffset; m_verts[i + 3].color = color;

//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "++++++++++++++++++++++++++++");
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "point ({}, {})", m_verts[i].position.x, m_verts[i].position.y);
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "point ({}, {})", m_verts[i+1].position.x, m_verts[i+1].position.y);
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "point ({}, {})", m_verts[i+2].position.x, m_verts[i+2].position.y);
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawBox()", "point ({}, {})", m_verts[i+3].position.x, m_verts[i+3].position.y);

        // vector.reserve is quick, but a vector.resize is typically faster
        // this is the part responsible for line drawing
        //m_indices.reserve(m_indices.size() + 8);
        m_indices.push_back(i);
        m_indices.push_back(i + 1);

        m_indices.push_back(i + 1);
        m_indices.push_back(i + 2);

        m_indices.push_back(i + 2);
        m_indices.push_back(i + 3);

        m_indices.push_back(i + 3);
        m_indices.push_back(i);
    }

    /**
     * @brief draw simple circle
     *
     * @param center x/y center of the circle
     * @param color RGBA value of color (0-255)
     * @param radius distance from center (size of circle)
     */
    void DebugRenderer::drawCircle(const glm::vec2& center, const glm::ivec4& color, float radius)
    {
        // circle = 2 * pi (radians)

        // good coding practice is to add more vertices as the size on the screen increases
        static const int NUM_VERTS = 10; //100;
        int start = m_verts.size();
        m_verts.resize(m_verts.size() + NUM_VERTS);
        for(int i = 0; i < NUM_VERTS; i++)
        {
            float angle = ((float)i / NUM_VERTS) * M_PI * 2.0f;
            // x = use cos(), y = use sin()
            m_verts[start + i].position.x = cos(angle) * radius + center.x;
            m_verts[start + i].position.y = sin(angle) * radius + center.y;
            m_verts[start + i].color = color;
        }

        // handle 0->1, 1->2, 2->3, etc indices
        m_indices.reserve(m_indices.size() + NUM_VERTS * 2);
        for(int i = 0; i < NUM_VERTS-1; i++)
        {
            m_indices.push_back(start + i);
            m_indices.push_back(start + i + 1);
        }

        // cover final point to start point for complete circle
        m_indices.push_back(start + NUM_VERTS - 1);
        m_indices.push_back(start);
    }

    /**
     * @brief draw simple point (useful for paths or line graphs)
     *
     * @param c vector of coordinates
     * @param color RGBA value of color (0-255)
     */
    void DebugRenderer::drawCoordinates(std::vector<glm::vec2> c, const glm::ivec4 color /*= COLOR_WHITE*/)
    {
		if(c.size() == 2) { drawCircle(c[0], color, c[1].x); } // ex: [0.0,1.0],[3.0,3.0]
		else
		{
			unsigned int numVerts = c.size();
			unsigned int i = m_verts.size(); // location of the first vertex that we're modifying
			m_verts.resize(i + numVerts); // make room for the four new vertices

			Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawCoordinates()", "^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
			// create vertices and add color
			for(unsigned int h = 0; h < numVerts; h++)
			{
				m_verts[i+h].position = glm::vec3(c[h], 0.0f);
				m_verts[i+h].color = color;
				Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::drawCoordinates()", "point ({}, {})", m_verts[i+h].position.x, m_verts[i+h].position.y);
			}

			// create indices
			m_indices.reserve( m_indices.size() + (numVerts * 2) );

			int tmpI = i;
			for(unsigned int h = 0; h < numVerts-1; h++)
			{
				m_indices.push_back(tmpI); // 0
				tmpI++; // +1
				m_indices.push_back(tmpI); // 1
			}

			// end to begining
			m_indices.push_back(tmpI); // 4
			m_indices.push_back(i); // end to start
        }
    }

    /**
     * @brief draw sets of passed coordinates (e.g. lines)
     *
     * @param vc vector of a collection (vector) of coordinates (a.k.a multiple separate lines)
     * @param color RGBA value of color (0-255)
     */
    void DebugRenderer::drawCoordinates(std::vector<std::vector<glm::vec2>> vc, const glm::ivec4 color /*= COLOR_WHITE*/)
    {
		for(unsigned i = 0; i < vc.size(); i++) { drawCoordinates(vc[i], color); }
    }

    /**
     * @brief draw single line from point A to point B
     *
     * this is usually called by drawCoordinates to plot actual lines
     *
     * @param pointA beginning x/y coordinates
     * @param pointB ending x/y coordinates
     * @param color RGBA value of color (0-255)
     */
    void DebugRenderer::drawLine(const glm::vec2& pointA, const glm::vec2& pointB, const glm::ivec4 color /*= COLOR_YELLOW*/)
    {
		int numVerts = 2;
        int i = m_verts.size(); // location of the first vertex that we're modifying
        m_verts.resize(i + numVerts); // make room for the two new vertices

        // create vertices and add color
        m_verts[i].position = glm::vec3(pointA, 0.0f);
		m_verts[i].color = color;
		m_verts[i+1].position = glm::vec3(pointB, 0.0f);
		m_verts[i+1].color = color;

        // create indices
        m_indices.reserve( m_indices.size() + (numVerts * 2) );
        int tmpI = i;
        for(int h = 0; h < numVerts-1; h++)
        {
            m_indices.push_back(tmpI); // 0
            tmpI++; // +1
            m_indices.push_back(tmpI); // 1
        }
    }

    /**
     * @brief draw a path of lines (coord A->B)
     *
     * @param c vector of coordinates in draw order
     * @param color RGBA value of color (0-255)
     */
    void DebugRenderer::drawPath(std::vector<glm::vec2> c, const glm::ivec4 color /*= COLOR_WHITE*/)
    {
        int numVerts = c.size();
        int i = m_verts.size(); // location of the first vertex that we're modifying
        m_verts.resize(i + numVerts); // make room for the four new vertices

        glm::ivec4 nodeColor = COLOR_ORANGE;
        if(nodeColor == color) { nodeColor = COLOR_WHITE; }

        // create vertices and add color
        for(unsigned int h = 0; h < numVerts; h++)
        {
            m_verts[i+h].position = glm::vec3(c[h], 0.0f);
            m_verts[i+h].color = color;
            drawCircle(c[h], nodeColor, 0.25f);
        }

        // create indices
        m_indices.reserve( m_indices.size() + (numVerts * 2) );

        int tmpI = i;
        for(int h = 0; h < numVerts-1; h++)
        {
            m_indices.push_back(tmpI); // 0
            tmpI++; // +1
            m_indices.push_back(tmpI); // 1
        }
    }

    /**
     * @brief place drawing data into buffer and wipe temporary vectors
     */
    void DebugRenderer::end()
    {
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::render()", "vao [{}], vbo [{}], verts.size() [{}], ibo [{}], indices.size [{}]",
//								m_vao, m_vbo, m_verts.size(), m_ibo, m_indices.size());
		glBindVertexArray(m_vao);
        glNamedBufferData(m_vbo, m_verts.size() * sizeof(DebugVertex), m_verts.data(), GL_DYNAMIC_DRAW);
        glNamedBufferData(m_ibo, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_DYNAMIC_DRAW);

        m_numElements = m_indices.size(); // two points per line
        m_indices.clear();
        m_verts.clear();
    }

    /**
     * @brief startup function to handle all openGL setup (buffers, vertex attributes, etc)
     */
    void DebugRenderer::init()
    {
		if(m_initialized) { return; }
        // shader initialization
        m_shaderProgram.compileShadersFromSource(DEBUG_RENDERER_VERT_SRC, DEBUG_RENDERER_FRAG_SRC);
        m_uniformProjection = glGetUniformLocation(m_shaderProgram.getProgramID(), uniformOne.c_str());
        if(m_uniformProjection == -1) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DebugRenderer::init()", "Failed to pull 'projection' uniform location!"); }

        // setup buffers
        glGenVertexArrays(1, &m_vao); // generate vertex array object
        glGenBuffers(1, &m_vbo); // generate vertex buffer object
        glGenBuffers(1, &m_ibo); // generate index buffer object

        // bind buffers
        glBindVertexArray(m_vao); // vertex array object encloses the vertex buffers and vertex attribute pointers
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo); // GL_ELEMENT_ARRAY_BUFFER is used for (index) drawing elements

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)offsetof(DebugVertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_INT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)offsetof(DebugVertex, color));

        glBindVertexArray(0); // unbinding
        m_initialized = true;
    }

    /**
     * @brief render the buffered elements
     *
     * @param lineWidth thickness of the line to draw
     */
    void DebugRenderer::render(float lineWidth)
    {
		if(!m_initialized) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "DebugRenderer::render(f)", "DebugRenderer has \033[91m\033[1mNOT\033[0m been initialized!"); return; }
        glLineWidth(lineWidth); // optional parameter to specify line width
        glBindVertexArray(m_vao);
        glDrawElements(GL_LINES, m_numElements, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    /**
     * @brief render buffered elements based on projection matrix
     *
     * @param projectionMatrix generated projection matrix (MVP model)
     * @param lineWidth thickness of the line to draw
     */
    void DebugRenderer::render(const glm::mat4& projectionMatrix, float lineWidth)
    {
		if(!m_initialized) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "DebugRenderer::render(ma4t, f)", "DebugRenderer has \033[91m\033[1mNOT\033[0m been initialized!"); return; }
		else if(m_shaderProgram.getProgramID() <= 0) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DebugRenderer::render(mat4, f)", "shaderProgram does not exist or is damaged!"); }
		else if(m_uniformProjection == -1) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DebugRenderer::render(mat4, f)", "uniformProject value is null!"); }

        glUseProgram(m_shaderProgram.getProgramID());
        glUniformMatrix4fv(m_uniformProjection, 1, GL_FALSE, &projectionMatrix[0][0]);

        //Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "DebugRenderer::render()", "num_elements [{}], lineWidth [{}], uniformProjection [{}]", m_numElements, lineWidth, m_uniformProjection);
        glLineWidth(lineWidth); // optional parameter to specify line width
        glBindVertexArray(m_vao);
        glLineWidth(lineWidth); // optional parameter to specify line width
        glDrawElements(GL_LINES, m_numElements, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    /**
     * @brief render buffered elements based on projection matrix, with a supplied new position
     *
     * @param projectionMatrix generated projection matrix (MVP model)
     * @param newPosition new position from which to adjust the projection matrix
     * @param lineWidth thickness of the line to draw
     */
	void DebugRenderer::render(const glm::mat4& projectionMatrix, const glm::vec2& newPosition, float lineWidth)
    {
		if(!m_initialized) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "DebugRenderer::render(mat4, vec2, f)", "DebugRenderer has \033[91m\033[1mNOT\033[0m been initialized!"); return; }
		else if(m_shaderProgram.getProgramID() <= 0) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DebugRenderer::render(mat4, vec2, f)", "shaderProgram does not exist or is damaged!"); }
		else if(m_uniformProjection == -1) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DebugRenderer::render(mat4, vec2, f)", "uniformProject value is null!"); }

        glUseProgram(m_shaderProgram.getProgramID());

        glm::mat4 newProj(1.0f);
        newProj = glm::translate(newProj, glm::vec3(newPosition, 0.0f));
        newProj = projectionMatrix * newProj;
        glUniformMatrix4fv(m_uniformProjection, 1, GL_FALSE, &projectionMatrix[0][0]);

        glLineWidth(lineWidth); // optional parameter to specify line width
        glBindVertexArray(m_vao);
        glDrawElements(GL_LINES, m_numElements, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    /// Private functions below ///////////////////////////////////////////////

    /**
     * @brief roating x/y point based on angle (radians)
     *
     * @param pos x/y position
     * @param angle degree of rotation (radians)
     * @return glm::vec2 the new location of the x/y coordinates
     */
    glm::vec2 DebugRenderer::rotatePoint(glm::vec2 pos, float angle)
    {
        glm::vec2 newVec;
        newVec.x = pos.x * cos(angle) - pos.y * sin(angle);
        newVec.y = pos.x * sin(angle) + pos.y * cos(angle);
        return newVec;
    }

    /**
     * @brief roating x/y point based on angle (radians), omitting z parameter
     *
     * @param pos x/y/z position, z is ignored
     * @param angle degree of rotation (radians)
     * @return glm::vec2 the new location of the x/y coordinates
     */
    glm::vec3 DebugRenderer::rotatePoint(glm::vec3 pos, float angle)
    {
        glm::vec3 newVec;
        newVec.x = pos.x * cos(angle) - pos.y * sin(angle);
        newVec.y = pos.x * sin(angle) + pos.y * cos(angle);
        newVec.z = pos.z;
        return newVec;
    }
}
