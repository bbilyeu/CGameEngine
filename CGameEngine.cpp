#include "CGameEngine.h"
#include "common/types.h"

/**
 * Used with openglCallbackFunction() to actually return output from OpenGL.
 *
 * @param source origin of the message (API, Window System, Shader Compiler, Third Party, Application, or Other/Unknown)
 * @param type classification for the message (Error, Undefined Behavior, etc)
 * @param id Unused at this time, but required as part of the standard
 * @param severity level of urgency (High, Low, Info, etc)
 * @param length Length (in GLsizei) of the 'message' parameter
 * @param message Actual message in the GL equivalent of char[]
 * @param userParam Unused at this time, but required as part of the standard
 */

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::string strSource, strType, strSeverity;
	int level = 0;
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             strSource = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   strSource = "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: strSource = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     strSource = "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     strSource = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           strSource = "Other"; break;
		default:							  strSource = "UNKNOWN SOURCE"; break;
	}

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               strType = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: strType = "Deprecated Behavior"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  strType = "Undefined Behavior"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         strType = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         strType = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              strType = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          strType = "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           strType = "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               strType = "Other"; break;
		default:							    strType = "UNKNOWN TYPE"; break;
	}

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			level = Logs::CRIT; break;
		case GL_DEBUG_SEVERITY_MEDIUM:
		case GL_DEBUG_SEVERITY_LOW:
			level = Logs::WARN; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			level = Logs::INFO; break;
		default:
			level = Logs::DEBUG; break;
	}

	Logger::getInstance().Log(level, Logs::Drawing, "GL MessageCallback", "'{}', '{}', Message: '{}'", strSource, strType, message);
}

namespace CGameEngine
{
	GLint major_v, minor_v;

	/**
	 *  Starts up SDL
	 */
    int init()
    {
        //SDL_Init(SDL_INIT_EVERYTHING); // initialize window (overkill)
        if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0)
        {
            //SDL_Log("CGameEngine::init() : Unable to initialize SDL [%s]", SDL_GetError());
			Logger::getInstance().Log(Logs::CRIT, Logs::Core, "CGameEngine::init()", "Unable to initialize SDL [{}]", SDL_GetError());
			return 98;
        }
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // standard double buffer
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

		Logger::getInstance().Log(Logs::DEBUG, "CGameEngine::init()", "init() completed");
        return 0;
    }

	/**
	 * Log/report a fatal SDL error and then exit
	 */
    void sdl_die(const char * message)
    {
		Logger::getInstance().Log(Logs::CRIT, Logs::Core, "CGameEngine::init()", "{}: {}", message, SDL_GetError());
        exit(2);
    }

	/**
	 * See MessageCallback() for full details
	 */
    void openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
    {
        (void)source; (void)type; (void)id; (void)severity; (void)length; (void)userParam;
		Logger::getInstance().Log(Logs::DEBUG, "CGameEngine::openglCallbackFunction()", "{}", message);
        if (severity==GL_DEBUG_SEVERITY_HIGH)
        {
           Logger::getInstance().Log(Logs::CRIT, Logs::Core, "CGameEngine::openglCallbackFunction()", "Aborting due to GL_DEBUG_SEVERITY_HIGH");
           abort();
        }
    }

	/**
	 * Enable or disable OpenGL debug
	 *
	 * @param enabled True to enable, false to disable
	 */
    void gameEngineGLDebug(bool enabled)
    {
        if(enabled)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

            glGetIntegerv(GL_MAJOR_VERSION, &major_v);
			glGetIntegerv(GL_MINOR_VERSION, &minor_v);

			Logger::getInstance().Log(Logs::INFO, Logs::Service, "gameEngineGLDebug()", "OpenGL Version: {}.{}", major_v, minor_v);
            if (major_v >= 4 && minor_v >= 3) { glDebugMessageCallback((GLDEBUGPROC)MessageCallback, 0); }
            else
            {
				glDebugMessageCallback((GLDEBUGPROC)openglCallbackFunction, nullptr);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
			}
        }
        else
        {
            glDisable(GL_DEBUG_OUTPUT);
            glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }
    }
}
