#ifndef CGAMEENGINE_H_INCLUDED
#define CGAMEENGINE_H_INCLUDED

#define GLEW_STATIC
#define GL_GLEXT_PROTOTYPES
//#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/glut.h>

/**
 * @file CGameEngine.h
 * @brief CGameEngine startup functions
 *
 *  init() starts SDL,
 *  openglCallbackFunction() is used for OpenGL error output,
 *  sdl_die() is for SDL fatal errors,
 *  gameEngineGLDebug() is used to enable/disable debug output.
 */
namespace CGameEngine
{
    extern int init();
    extern void openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
    extern void sdl_die(const char * message);
    extern void gameEngineGLDebug(bool enabled);
}

#endif // CGAMEENGINE_H_INCLUDED


/// Process:
/*
 CGAMEENGINE      init() :: First call in MainGame::init to load SDL
 AudioEngine      init() :: Second call in MainGame::init to load audio engine
 Window           create("Window Name", scrWidth, scrHeight, currFlags) in MainGame::init to create root window
 Camera           init(scrWidth, scrHeight) in MainGame::init to create camera
 Camera           setPosition(scrWidth / 2.0f, scrHeight / 2.0f) in MainGame::init to center camera on screen
 SpriteBatch      init() in MainGame::init to create the initial vertex array
 GLSLProgram      defaultShaders() :: in MainGame::init to load hardcoded shaders (or manually build load them in from files)
 Timing           setMaxFPS(60.0f) :: in MainGame::init to set default maxFPS
 Timing           FPSLimiter :: in MainGame::GameLoop beginFrame() and endFrame() for FPS (setFPS in MainGame::init)

 [DRAWING // MainGame::draw]
    glClearDepth(1.0);  // set base depth to 1.0
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear color/depth buffers
    m_textureProgram.use(); // activate GLSLProgram
    glActiveTexture(GL_TEXTURE0);   // use texture 0
    GLint textureUniform = m_textureProgram.getUniformLocation("image");    // load uniform from texture program (GLSLProgram)
    glUniform1i(textureUniform, 0); // bind uniform

    // camera bits
    glm::mat4 projectionMatrix = m_camera.getCameraMatrix();    // Camera2D
    GLint pUniform = m_textureProgram.getUniformLocation("projection");  // load uniform from texture program (GLSLProgram)
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]); // bind the uniform

    // spritebatch bits
    m_spriteBatch.begin();
    ### DRAW OBJECTS USING OBJECT FUNCTIONS ###
    m_spriteBatch.end();
    m_spriteBatch.renderBatch();

    m_textureProgram.unuse();
 [END DRAWING // MainGame::draw]


*/
