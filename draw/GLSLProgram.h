#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H
#define GL_GLEXT_PROTOTYPES
#include <string>
#include <common/types.h>
//#include <GLES3/gl3.h>
//#include <GL/glew.h>
//#include <GL/glcorearb.h>
/*#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
*/
#include "draw/Shaders.h"

// GLSL = (Open)GL Shading Language

namespace CGameEngine
{
    class GLSLProgram
    {
        public:
            GLSLProgram();
            ~GLSLProgram();
            GLSLProgram(const GLSLProgram& g) { copy(*this, g); }
            GLSLProgram& operator=(const GLSLProgram& g) { copy(*this, g); return *this; }
            friend void copy(GLSLProgram& dst, const GLSLProgram& src);
            GLuint getUniformLocation(const std::string& uniformName);
            const GLuint& getProgramID() const { return m_programID; }
            const bool& isInUse() const { return m_isInUse; }
            void addAttribute(const std::string& attribName); // bind & count attributes
            void compileShaders(const std::string& vertexShaderFilepath, const std::string& fragmentShaderFilepath); // compile shaders from file
            void compileShadersFromSource(const char* vertexSource, const char* fragmentSource);
            void defaultGUIShaders();
            void defaultShaders(); // standard shaders loaded
            void dispose(); // same as deconstructor, but called on command
            void linkShaders(); // link shaders to program
            void use(); // to bind and enable opengl 'program'
            void unuse(); // to unbind and disable opengl 'program'

        private:
            GLuint m_programID = 0;
            GLuint m_vertexShaderID = 0;
            GLuint m_fragmentShaderID = 0;

            bool compileShader(const char* source, GLuint id);

            bool m_isInUse = false;
            int m_numAttrib = 0; // count number of attributes
    };
}
#endif // GLSLPROGRAM_H
