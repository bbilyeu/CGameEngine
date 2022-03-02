#include "draw/GLSLProgram.h"

#include "srv/IOManager.h"
#include <fstream>
#include <vector>
#include "srv/Logger.h"

namespace CGameEngine
{
    /*
        W = S * R * T
        W is the world transform
        S is scaling.
        R is rotation
        T is translation
    */

    GLSLProgram::GLSLProgram() {} // empty

    GLSLProgram::~GLSLProgram() { dispose(); } // empty

    void copy(GLSLProgram& dst, const GLSLProgram& src)
    {
        if(&dst != &src)
        {
            dst.m_programID = src.m_programID;
            dst.m_vertexShaderID = src.m_vertexShaderID;
            dst.m_fragmentShaderID = src.m_fragmentShaderID;
            dst.m_isInUse = src.m_isInUse;
            dst.m_numAttrib = src.m_numAttrib;
        }
    }

    /**
     * @brief attempt to grab glUniformLocation
     * 
     * failing to find it will result in a fatal error
     * 
     * @param uniformName string name of the uniform to grab
     * @return GLuint the ID of the uniform
     */
    GLuint GLSLProgram::getUniformLocation(const std::string& uniformName)
    {
        GLuint location = glGetUniformLocation(m_programID, uniformName.c_str());
        if(location == GL_INVALID_INDEX)
        {
           Logger::getInstance().Log(Logs::FATAL, Logs::GLSLProgram, "GLSLProgram::getUniformLocation()", "Uniform '{}' not found in shader! (Location = '{}')", uniformName, location);
        }
        return location;
    }

    /**
     * @brief add attribute via glBindAttribLocation
     * 
     * @param attribName passed attribute name
     */
    void GLSLProgram::addAttribute(const std::string& attribName)
    {
        glBindAttribLocation(m_programID, m_numAttrib++, attribName.c_str());
    }

    /**
     * @brief compile shaders from files
     * 
     * @param vertexShaderFilepath file path to vertex shader
     * @param fragmentShaderFilepath file path to fragment shader
     */
    void GLSLProgram::compileShaders(const std::string& vertexShaderFilepath, const std::string& fragmentShaderFilepath)
    {
        std::string vertSource; // vertex shader source code as a a string
        std::string fragSource; // fragment shader source code as a string

        IOManager::readFileToBuffer(vertexShaderFilepath, vertSource);
        IOManager::readFileToBuffer(fragmentShaderFilepath, fragSource);

        compileShadersFromSource(vertSource.c_str(), fragSource.c_str());
    }

    /**
     * @brief compile shaders from char* arrays (data, instead of file)
     * 
     * @param vertexSource vertex shader data
     * @param fragmentSource fragment shader data
     */
    void GLSLProgram::compileShadersFromSource(const char* vertexSource, const char* fragmentSource)
    {
        m_programID = glCreateProgram();

        m_vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        if(!m_vertexShaderID) { Logger::getInstance().Log(Logs::FATAL, Logs::GLSLProgram, "GLSLProgram::compileShadersFromSource()", "Vertex shader failed to be created!"); }
        m_fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        if(!m_fragmentShaderID) { Logger::getInstance().Log(Logs::FATAL, Logs::GLSLProgram, "GLSLProgram::compileShadersFromSource()", "Fragment shader failed to be created!"); }

        if(!compileShader(vertexSource, m_vertexShaderID)) { Logger::getInstance().Log(Logs::FATAL, Logs::GLSLProgram, "GLSLProgram::compileShader()", "Vertex Shader failed to compile!"); }
        if(!compileShader(fragmentSource, m_fragmentShaderID)) { Logger::getInstance().Log(Logs::FATAL, Logs::GLSLProgram, "GLSLProgram::compileShader()", "Fragment Shader failed to compile!"); }
        linkShaders();
    }

    /**
     * @brief compile default vertex and fragment shaders used by GUIs
     */
    void GLSLProgram::defaultGUIShaders()
    {
        // load generic gui shaders from source to get moving faster
        compileShadersFromSource(DEFAULT_GUI_VERT_SRC, DEFAULT_FRAG_SRC);
        linkShaders();
    }

    /**
     * @brief compile default shaders with no special components
     */
    void GLSLProgram::defaultShaders()
    {
        // load generic shaders from source to get moving faster
        compileShadersFromSource(DEFAULT_VERT_SRC, DEFAULT_FRAG_SRC);
        linkShaders();
    }

    /**
     * @brief safely close via glDeleteProgram
     */
    void GLSLProgram::dispose()
    {
        if(m_programID) { glDeleteProgram(m_programID); }
    }

    /**
     * @brief attach the shaders to the GLSL program, link them, then delete the temporary shader links
     */
    void GLSLProgram::linkShaders()
    {
        // attach shaders to program
        glAttachShader(m_programID, m_vertexShaderID);
        glAttachShader(m_programID, m_fragmentShaderID);

        // link our program
        glLinkProgram(m_programID);

        // error checking and output display
        GLint isLinked;
        glGetProgramiv(m_programID, GL_LINK_STATUS, &isLinked);
        if(!isLinked)
        {
            GLint maxLength = 0;
            glGetProgramiv(m_programID , GL_INFO_LOG_LENGTH, &maxLength);

            //std::vector<char> errorLog(maxLength);
            char* errorLog = new char[maxLength];
            //std::vector<GLchar> errorLog(maxLength);
            glGetProgramInfoLog(m_programID, maxLength, &maxLength, errorLog);
            glDeleteProgram(m_programID); // delete unused program
            glDeleteShader(m_vertexShaderID); // delete shaders!
            glDeleteShader(m_fragmentShaderID); // delete shaders!

			Logger::getInstance().Log(Logs::FATAL, Logs::GLSLProgram, "GLSLProgram::linkShaders()", "Linking shaders failed! Error: {}", errorLog); // produce output
        }

        // detach and delete shaders
        //glDetachShader(m_programID, m_vertexShaderID);
        //glDetachShader(m_programID, m_fragmentShaderID);
        glDeleteShader(m_vertexShaderID);
        glDeleteShader(m_fragmentShaderID);
    }

    /**
     * @brief use the GLSL program, enabling vertex attributes (if applicable)
     */
    void GLSLProgram::use()
    {
        glUseProgram(m_programID);
        for(int i = 0; i < m_numAttrib; i++)
        {
            glEnableVertexAttribArray(i);
        }
        m_isInUse = true;
    }

    /**
     * @brief unuse the GLSL program, disabling vertex attributes (if applicable)
     */
    void GLSLProgram::unuse()
    {
        glUseProgram(0);
        for(int i = 0; i < m_numAttrib; i++)
        {
            glDisableVertexAttribArray(i);
        }
        m_isInUse = false;
    }

    /// Private functions below here //////////////////////////////////////////

    /**
     * @brief perform actual glCompileShader
     * 
     * @param source source data for a shader
     * @param id internal openGL ID
     * @return true successfully compiled
     * @return false failed to compile, critical error
     */
    bool GLSLProgram::compileShader(const char* source, GLuint id)
    {
		// GLenum target is handled in compileShadersFromSource()
        glShaderSource(id, 1, &source, nullptr); // sourcing in shader
        glCompileShader(id); // actually compiling it

        // error checking to ensure we got it correctly
        GLint success = GL_FALSE;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength); // get debug output size (in char)

            //std::vector<char> errorLog(maxLength); // define vector of char (string...ish) to hold it
            char* errorLog = new char[maxLength];
            glGetShaderInfoLog(id, maxLength, &maxLength, errorLog); // collect actual debug output
            glDeleteShader(id); // delete shader!
            //std::printf("%s\n", &errorLog[0]);
            Logger::getInstance().Log(Logs::CRIT, Logs::GLSLProgram, "GLSLProgram::compileShader()", "Error: {}", &errorLog[0]); // produce error
            return false;
        }
        return true;
    }
}
