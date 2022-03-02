#ifndef RENDERER2D_H
#define RENDERER2D_H
#include "common/types.h"
#include "draw/draw_common.h"

// these are test values
#define TEXTURES_MAX 1000
#define PARTICLE_MAX 1000

#define BASE_INDEX_COUNT 6

/**
 * @file Renderer2D.h
 * @brief Singleton class for the 2D renderer
 *
 *
 *	draw - Standard, short-lived asset
 *		* Use the "new hotness" (bindless with a persistently mapped buffer)
 *	longDraw - Longer lived asset (e.g. player model, city NPCs)
 *		* ???
 *	staticDraw - Items that do not go away (e.g. terrain)
 *		* Use the "old" buffer bind as it needs to be bound and buffered once, then it will draw every time
 *
 *	Ref:
 *		https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
 *		https://litasa.github.io/blog/2017/09/04/OpenGL-MultiDrawIndirect-with-Individual-Textures
 *		https://www.reddit.com/r/opengl/comments/3m9u36/how_to_render_using_glmultidrawarraysindirect/
 *		https://www.g-truc.net/post-0518.html
 *		https://github.com/fenbf/GLSamples/blob/master/src/PersistentMappedBuffer.cpp (persistently mapped buffers)
 *		--->https://gamedev.stackexchange.com/questions/104310/opengl-4-5-primitive-restart-vs-base-index/104314#104314
 *
 *	Notes:
 *		The final three parameters of DrawElementsIndirectCommand (firstIndex, baseVertex, and baseInstance) allow for
 *		buffer sharing. Allowing for suballocating between vertex and index buffers.
*/

/// \TODO: Accept actual screen width/height

namespace CGameEngine
{
	class GLSLProgram;
	class BufferLockManager;

	static uint32_t GetApiError()
	{
		#if defined(_DEBUG)
			return glGetError();
		#else
			return GL_NO_ERROR;
		#endif
	}

	static const Vertex baseQuadVertices[] =
    {
		//   			x      y     z      		u     v
        { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f) },
        { glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f) },
        { glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 1.0f) },
        { glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(0.0f, 1.0f) },
    };

    static const GLuint baseQuadIndices[] = { 0, 1, 2, 0, 2, 3 };

    namespace DrawPathType { enum LEVELS { NONE, STATIC, LONG, FAST, INVALID }; }

	struct DrawPath
	{
		// opengl bound or buffered variables
		GLuint pathType = DrawPathType::NONE;
		GLuint vertexArrayObject;
		GLuint vertexBuffer;
		std::vector<Vertex> vertices;
		GLuint indexBuffer;
		std::vector<GLuint> indices;
		GLuint transformBuffer;
		std::vector<glm::mat4> transforms;
		GLuint textureAddressBuffer;
		std::vector<TextureAddress> textureAddresses;
		GLuint drawIDBuffer;
		std::vector<GLuint> drawIDs;
		GLuint drawCmdBuffer;
//		std::vector<DEIC> drawCommands;
		std::vector<DAIC> drawCommands;

		// non-buffered variables
		std::vector<Glyph*> glyphs;

		// persistently mapped buffer variables (ivec2[head, size])
//		RingBufferHead <DEIC*>rbCommands = { 0, 0, (3 * PARTICLE_MAX * sizeof(DEIC)), 0 };
		RingBufferHead <DAIC*>rbCommands = { 0, 0, (3 * PARTICLE_MAX * sizeof(DAIC)), 0 };
		RingBufferHead <glm::mat4*>rbTransforms = { 0, 0, (3 * PARTICLE_MAX * sizeof(glm::mat4)), 0 };
		RingBufferHead <Vertex*>rbVertices = { 0, 0, (3 * PARTICLE_MAX * (sizeof(Vertex))), 0 };

		// state changes
		bool updated = false;

		// comparison
		bool operator!=(const DrawPath& other) { return !(*this == other); }
		bool operator==(const DrawPath& other) { return (vertexArrayObject == other.vertexArrayObject &&
														 vertexBuffer == other.vertexBuffer &&
														 indexBuffer == other.indexBuffer); }

		// cleanup (for fast draw)
		void clear(bool glyphsToo = false)
		{
			vertices.clear();
			indices.clear();
			transforms.clear();
			textureAddresses.clear();
			drawIDs.clear();
			drawCommands.clear();
			if(glyphsToo) { while(!glyphs.empty()) { safeDelete(glyphs.back()); glyphs.pop_back(); } }
		}
	};

	class Renderer2D
	{
		public:
			static Renderer2D& getInstance()
			{
				static Renderer2D instance;
				return instance;
			}

			bool init(int scrWidth = 1280, int scrHeight = 960);
			void draw(TextureAddress tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle = 0.0f, bool isCentered = false);
			Glyph* addLongDraw(TextureAddress tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle = 0.0f, bool isCentered = false);
			void addStaticDraw(TextureAddress tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle = 0.0f, bool isCentered = false);
			Glyph* targetedDraw(DrawPath* p, const TextureAddress& tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle = 0.0f, bool isCentered = false);
			void begin(const glm::mat4& cameraMatrix); // pre-drawing setup
			void end(); // post-drawing wrap-up (state reset)
			void renderPrep(DrawPath* p); // targeted begin() & end()
			void render(); // render to screen
			void renderDrawPath(DrawPath* p, GLuint glProgID = 0);
			void setupDrawPath(DrawPath* p);
			void setCameraMatrix(const glm::mat4& cameraMatrix, GLuint glslProgID);
			void setWindowDimensions(float scrWidth, float scrHeight);

			static bool compareBackToFront(Glyph* a, Glyph* b) { return (a < b); }
            static bool compareFrontToBack(Glyph* a, Glyph* b) { return (a > b); }

			// lazy conversions to say MUCH coding
//			void draw(const TextureAddress& tex, const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle = 0.0f)
//				{ draw(tex, glm::vec3(position.x, position.y, 0.0f), dimensions, textureCoordinates, angle); }
//			Glyph* addLongDraw(const TextureAddress& tex, const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle = 0.0f)
//				{ return addLongDraw(tex, glm::vec3(position.x, position.y, 0.0f), dimensions, textureCoordinates, angle); }
//			void addStaticDraw(const TextureAddress& tex, const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle = 0.0f)
//				{ addStaticDraw(tex, glm::vec3(position.x, position.y, 0.0f), dimensions, textureCoordinates, angle); }
//			Glyph* targetedDraw(DrawPath* p, const TextureAddress& tex, const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle = 0.0f)
//				{ return targetedDraw(p, tex, glm::vec3(position.x, position.y, 0.0f), dimensions, textureCoordinates, angle); }

		private:
			Renderer2D();
			virtual ~Renderer2D();
			std::vector<TextureAddress> m_texAddresses;
			std::vector<glm::mat4> m_transforms;
			DrawPath m_fastDrawPath; // temp/one-time
			DrawPath m_longDrawPath; // characters, equipment, common npcs
			DrawPath m_staticDrawPath = DrawPath(); // terrain, background(s)
			std::vector<DrawPath*> m_drawPaths;
			BufferLockManager* m_bufferLockManager = nullptr;
			GLSLProgram* m_prog = nullptr;
			bool m_beginCalled = false;
			bool m_useShaderDrawParameters = false;
			bool m_initDone = false;
			int m_textureCount = 0;
			int m_screenHeight = 0;
			int m_screenWidth = 0;

            // state saving
            GLboolean m_cullFace;
            GLint m_cullFaceMode;
            GLint m_frontFace;
            GLboolean m_scissorTest;
            GLboolean m_blend;
            GLboolean m_depthTest;
            GLboolean m_depthMask;
            GLint m_uniformMVP = -1;

            GLbitfield m_persistentMapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			GLbitfield m_persistentCreateFlags = m_persistentMapFlags | GL_DYNAMIC_STORAGE_BIT;
            const std::string uniformOne = "ViewProjection";

			Glyph* createGlyph(DrawPath* path, const TextureAddress& tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle, bool isCentered);
	};

}
#endif // RENDERER2D_H
