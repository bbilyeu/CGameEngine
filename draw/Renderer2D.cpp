#include "draw/Renderer2D.h"
#include <stddef.h>
#include "common/glm_util.h"
#include "common/util.h"
#include "draw/GLSLProgram.h"
#include "draw/BufferLock.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

/// \TODO: Write logic to handle irregular objects' indices/vertices count

namespace CGameEngine
{
	/**
	 * @brief init all rendering-related classes, setup default drawpaths
	 *
	 * @param scrWidth screen width in pixels
	 * @param scrHeight screen height in pixels
	 * @return true successfully started renderer
	 * @return false failed to start renderer, critical (or fatal) failure
	 */
	bool Renderer2D::init(int scrWidth /*= 1280*/, int scrHeight /*= 960*/)
	{
		if(!m_initDone)
		{
			// init components
			m_bufferLockManager = new BufferLockManager(false);
			m_prog = new GLSLProgram();
			m_prog->compileShadersFromSource(V450_VERT_SRC, V450_FRAG_SRC);
			m_uniformMVP = glGetUniformLocation(m_prog->getProgramID(), uniformOne.c_str());
			if(m_uniformMVP == -1) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "Renderer2D::init()", "uniformMVP is -1!"); }
			setWindowDimensions(scrWidth, scrHeight);

			m_staticDrawPath.pathType = DrawPathType::STATIC;
			setupDrawPath(&m_staticDrawPath);
			m_longDrawPath.pathType = DrawPathType::LONG;
			setupDrawPath(&m_longDrawPath);
			m_fastDrawPath.pathType = DrawPathType::FAST;
			setupDrawPath(&m_fastDrawPath);
			m_initDone = true;
		}

		// return success/fail
		return m_initDone;
	}

	/**
	 * @brief add a one-use glyph to draw to the fast draw path (sprites, particles, etc)
	 *
	 * @param tex texture address (container handle and slice number)
	 * @param position x/y/z position to draw the object
	 * @param dimensions width and height of the object
	 * @param textureCoordinates a.k.a UV coordinates
	 * @param angle degree of rotation (radians)
	 * @param isCentered is the position already the center of the drawing spot
	 */
	void Renderer2D::draw(TextureAddress tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle /*= 0.0f*/, bool isCentered /*= false*/)
	{
		if(!tex.exists()) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::draw()", "Passed TextureAddress pointer was null."); return; }
		createGlyph(&m_fastDrawPath, tex, position, dimensions, textureCoordinates, angle, isCentered);
	}

	/**
	 * @brief add a medium-life glyph to draw to the long draw path (NPCs, characters, etc)
	 *
	 * @param tex texture address (container handle and slice number)
	 * @param position x/y/z position to draw the object
	 * @param dimensions width and height of the object
	 * @param textureCoordinates a.k.a UV coordinates
	 * @param angle degree of rotation (radians)
	 * @param isCentered is the position already the center of the drawing spot
	 */
	Glyph* Renderer2D::addLongDraw(TextureAddress tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle /*= 0.0f*/, bool isCentered /*= false*/)
	{
		if(!tex.exists()) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::addLongDraw()", "Passed TextureAddress pointer was null."); return nullptr; }
		return createGlyph(&m_longDrawPath, tex, position, dimensions, textureCoordinates, angle, isCentered); // return the newly created glyph
	}

	/**
	 * @brief add a 'permanent' glyph to draw to the static draw path (terrain, background bitmaps)
	 *
	 * @param tex texture address (container handle and slice number)
	 * @param position x/y/z position to draw the object
	 * @param dimensions width and height of the object
	 * @param textureCoordinates a.k.a UV coordinates
	 * @param angle degree of rotation (radians)
	 * @param isCentered is the position already the center of the drawing spot
	 */
	void Renderer2D::addStaticDraw(TextureAddress tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle /*= 0.0f*/, bool isCentered /*= false*/)
	{
		if(!tex.exists()) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::addStaticDraw()", "Passed TextureAddress pointer was null."); return; }
		createGlyph(&m_staticDrawPath, tex, position, dimensions, textureCoordinates, angle, isCentered);
	}

	/**
	 * @brief add a glyph to draw to the passed draw path
	 *
	 * @param p pointer to the draw path to which the glyph will be added
	 * @param tex texture address (container handle and slice number)
	 * @param position x/y/z position to draw the object
	 * @param dimensions width and height of the object
	 * @param textureCoordinates a.k.a UV coordinates
	 * @param angle degree of rotation (radians)
	 * @param isCentered is the position already the center of the drawing spot
	 */
	Glyph* Renderer2D::targetedDraw(DrawPath* p, const TextureAddress& tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle /*= 0.0f*/, bool isCentered /*= false*/)
	{
		Glyph* retVal = nullptr;
		if(!p) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::targetedDraw()", "Passed TextureAddress pointer was null."); }
		else if(!tex.exists()) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::targetedDraw()", "Passed TextureAddress object was empty."); }
		else if(p->pathType == DrawPathType::NONE || p->pathType >= DrawPathType::INVALID) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::targetedDraw", "Passed DrawPath has an invalid pathType!"); }
		else
		{
			retVal = createGlyph(p, tex, position, dimensions, textureCoordinates, angle, isCentered);
			if(retVal) { p->updated = true; }
			if(p->pathType != DrawPathType::LONG) { retVal = nullptr; }
		}

		return retVal;
	}

	/**
	 * @brief start of a drawing frame, begin accepting draw() calls
	 *
	 * @param cameraMatrix the passed camera matrix (for scale/position)
	 */
	void Renderer2D::begin(const glm::mat4& cameraMatrix)
	{
		//Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Renderer2D::begin()", "\033[1mbegin() start\033[0m");
		if(m_beginCalled) { Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "Renderer2D::begin", "begin() already called."); return; }
		else { m_beginCalled = true; m_fastDrawPath.updated = true; }

		/// backup states before proceeding
		m_cullFace = glIsEnabled(GL_CULL_FACE);
		glGetIntegerv(GL_CULL_FACE_MODE, &m_cullFaceMode);
		glGetIntegerv(GL_FRONT_FACE, &m_frontFace);
		m_scissorTest = glIsEnabled(GL_SCISSOR_TEST);
		m_blend = glIsEnabled(GL_BLEND);
		//m_depthTest = glIsEnabled(GL_DEPTH_TEST);
		glGetBooleanv(GL_DEPTH_TEST, &m_depthTest);
		glGetBooleanv(GL_DEPTH_WRITEMASK, &m_depthMask);

        /// setup view
        //glUseProgram(m_prog->getProgramID());
		//glUniformMatrix4fv(m_uniformMVP, 1, GL_FALSE, &cameraMatrix[0][0]);
		setCameraMatrix(cameraMatrix, m_prog->getProgramID());

		/////////////////////	non functional	///////////////////////////////
		//		/// rasterizer state
		//		glEnable(GL_CULL_FACE);
		//		glCullFace(GL_FRONT);
		//		glFrontFace(GL_CCW);
		//		glDisable(GL_SCISSOR_TEST);
		//
		//		/// blending state
		//		glDisable(GL_BLEND);
		//		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//
		//		/// depth stencil state
		//		glEnable(GL_DEPTH_TEST);
		//		glDepthMask(GL_TRUE);
		/////////////////////	non functional	///////////////////////////////
	}

	/**
	 * @brief end the drawing phase, prepare to render
	 */
	void Renderer2D::end()
	{
		if(!m_beginCalled) { Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "Renderer2D::end", "begin() has not been called or end() has been called already."); return; }
		else { m_beginCalled = false; }

		renderPrep(&m_staticDrawPath);
		renderPrep(&m_longDrawPath);
		renderPrep(&m_fastDrawPath);
	}

	/**
	 * @brief prepare the passed draw path for rendering by handling all necessary functions
	 *
	 * prepare to render a draw path by performing necessary evils such as: ring buffer management,
	 * cleaning up old glyphs (if applicable), updating the numerous vectors (verts, indices, etc),
	 *
	 * @param p passed draw path
	 */
	void Renderer2D::renderPrep(DrawPath* p)
	{
		if(!p) { Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "Renderer2D::renderPrep", "nullptr passed for draw path."); }
        else if(p->vertexArrayObject == 0) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::renderPrep", "Passed draw path does *NOT* have a valid vertex array object!"); }
        else if(p->pathType == DrawPathType::NONE || p->pathType >= DrawPathType::INVALID) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::renderPrep", "Passed DrawPath has an invalid pathType!"); }
        else if(p->glyphs.size() <= 0) { /*Logger::getInstance().Log(Logs::VERBOSE, Logs::Drawing, "Renderer2D::renderPrep", "Passed DrawPath has no glyphs to prep!");*/ }
		else
		{
			/// create batches (where applicable)
			std::vector<Vertex>* verts = nullptr;
			std::vector<GLuint>* indices = nullptr;
			std::vector<glm::mat4>* transforms = nullptr;
			//std::vector<DEIC>* DEICs = nullptr;
			std::vector<DAIC>* DAICs = nullptr;
			std::vector<TextureAddress>* texAddrs = nullptr;
			std::vector<Glyph*>* glyphs = nullptr;
			TextureAddress activeTexture = {0,0};
			glm::mat4 identMat = glm::mat4(1.0f);
			GLuint inc = 0, failed = 0, indexOffset = 0, vertCount = 0;

			if(p->updated)
			{
				p->updated = false;

				// wipe old data
				p->clear();
				inc = 0;
				failed = 0;
				indexOffset = 0;

				// sort glyphs based on texture address, then slice/page
				std::stable_sort(p->glyphs.begin(), p->glyphs.end(), compareFrontToBack);

				// assign lazy handles
				//DEICs = &p->drawCommands;
				DAICs = &p->drawCommands;
				verts = &p->vertices;
				indices = &p->indices;
				texAddrs = &p->textureAddresses;
				transforms = &p->transforms;
				glyphs = &p->glyphs;
				std::vector<Vertex>* preVerts = nullptr;

				/// \TODO: READ IN ACTUAL INDICES
				//for(int in = 0; in < QUAD_VERTS; in++) { indices->push_back(baseQuadIndices[in]); }

				for(unsigned int i = 0; i < glyphs->size(); i++)
				{
					auto it = glyphs->at(i);

					// ensure we have a valid texture address
					if(!it->textureAddress.defined())
					{
						Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::drawPrep()", "Iteration [{}] of [{}] has a null texture address (0,0)!", i, glyphs->size());
						failed++;
					}
					else
					{
						indexOffset = verts->size();

						for(int in = 0; in < BASE_INDEX_COUNT; in++) { indices->push_back(baseQuadIndices[in] + indexOffset); }
						for(auto& v : it->vertices) { verts->push_back(v); }

						// creating our model space to world space matrix ('model' in "projection * view * model")
						glm::mat4 transRotate = glm::rotate(identMat, glm::radians(it->angle), glm::vec3(0.0f, 0.0f, 1.0f));
						transforms->push_back(transRotate);
						transforms->back() = glm::translate(transforms->back(), it->position);
						//transforms->push_back(transforms->back());

//						Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Renderer2D::drawPrep()", "activeTexture [{},{}] vs it->textureAddress [{},{}]",
//							activeTexture.containerHandle, activeTexture.sliceNumber, it->textureAddress.containerHandle, it->textureAddress.sliceNumber);
//						if(preVerts != nullptr) { Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Renderer2D::drawPrep()", "Are preVerts and it->vertices identical? [{}]", std::equal(preVerts->begin(), preVerts->end(), it->vertices.begin())); }

						// update previous draw command or create a new one
						if(activeTexture == it->textureAddress && preVerts != nullptr &&
							std::equal(preVerts->begin(), preVerts->end(), it->vertices.begin()))
						{
							// append previous draw command
							DAICs->back().instanceCount += 1;
							//DAICs->back().vertexCount += BASE_INDEX_COUNT;
						}
						else
						{
							// different texture or dimensions, new draw command

							// ghetto add indices
							//for(int in = 0; in < BASE_INDEX_COUNT; in++) { indices->push_back(baseQuadIndices[in]); } // + offset); }
							//indices->push_back(0 + offset); indices->push_back(1 + offset); indices->push_back(2 + offset); indices->push_back(3 + offset);
							// vertices
//							for(auto& v : it->vertices) { verts->push_back(v); }
							// create draw command
//							DEIC tmp = { QUAD_VERTS, 1, 0, 0, 0 };
//							DEICs->push_back(tmp);
							DAIC tmp = { BASE_INDEX_COUNT, 1, (inc * BASE_INDEX_COUNT), i };
							DAICs->push_back(tmp);
							activeTexture = it->textureAddress;
//							texAddrs->push_back(currentTex);
//							p->drawIDs.push_back(inc);
							inc++;
						}

						/// \NOTE: Current issue is that the draw command is only drawing one object, in two iterations.
						///		This is responsible for the blank second box
//						DEIC tmp = { BASE_INDEX_COUNT, 1, 0, 0, 0 };
//						DEICs->push_back(tmp);
//						for(auto& v : it->vertices) { verts->push_back(v); }
//						for(int in = 0; in < BASE_INDEX_COUNT; in++) { indices->push_back(baseQuadIndices[in]); }
//						texAddrs->push_back(it->textureAddress);
//						p->drawIDs.push_back(i);
//						DAIC tmp = { QUAD_VERTS, 1, (i*BASE_INDEX_COUNT), 0 };
//						DAICs->push_back(tmp);
//						for(auto& v : it->vertices) { verts->push_back(v); }
//						for(int in = 0; in < BASE_INDEX_COUNT; in++) { indices->push_back(baseQuadIndices[in] + offset); }
//						texAddrs->push_back(it->textureAddress);
//						p->drawIDs.push_back(i);

//						Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Renderer2D::drawPrep()",
//							"\n\033[93mDAIC #{}\033[0m:\n\tvertCount\t\t{}\n\tinstCount\t\t{}\n\tfirstVert\t\t{}\n\tbaseInst\t\t{}\n",
//							DAICs->size(), DAICs->back().vertexCount, DAICs->back().instanceCount, DAICs->back().firstVertex, DAICs->back().baseInstance);


						texAddrs->push_back(activeTexture);
						p->drawIDs.push_back(i);
//						inc++;
					}

					preVerts = &it->vertices;
				}

				// checks to ensure data was created
				if(transforms->size() != p->drawIDs.size())
					{ Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::renderPrep()", "transforms.size() [{}] does not equal drawIDs.size() [{}]", transforms->size(), p->drawIDs.size()); }
				else if(verts->empty()) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::renderPrep()", "verts is empty!"); return; }
				else if(glyphs->empty()) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::renderPrep()", "glyphs is empty!"); return; }

				/// HUGE DEBUG BLOCK
				if(true)
				{
					std::string tmp;
					for(int i = 0; i < verts->size(); i++)
					{
					tmp = tmp + " " + std::to_string(verts->at(i).position.x) + ", " + std::to_string(verts->at(i).position.y) + " \033[1m||\033[0m";
					if (((i+1) != verts->size()) && ((i+1) % 6 == 0)) { tmp+="\n"; }
					}
					Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "----", "verts:\n {}", tmp);
					tmp = "";
					for(int i = 0; i < indices->size(); i++) { tmp = tmp + " " + std::to_string(indices->at(i)) + ", "; }
					Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "----", "indices: {}", tmp);
					tmp = "";
					for(int i = 0; i < p->drawIDs.size(); i++) { tmp = tmp + " " + std::to_string(p->drawIDs[i]) + ", "; }
					Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "----", "drawIDs: {}", tmp);
					tmp = "";

					for(int i = 0; i < transforms->size(); i++)
					{
						glm::mat4 xform = transforms->at(i);
						Logger::getInstance().Log(Logs::DEBUG, "----",
							"transform #{}\n(\t{},\t{},\t{},\t{}\t)\n(\t{},\t{},\t{},\t{}\t)\n(\t{},\t{},\t{},\t{}\t)\n(\t{},\t{},\t{},\t{}\t)\n",
							i,
							xform[0][0], xform[0][1], xform[0][2], xform[0][3],
							xform[1][0], xform[1][1], xform[1][2], xform[1][3],
							xform[2][0], xform[2][1], xform[2][2], xform[2][3],
							xform[3][0], xform[3][1], xform[3][2], xform[3][3] );
					}
				}

				/// verts
				if(p->pathType != DrawPathType::FAST) { glNamedBufferData(p->vertexBuffer, sizeof(Vertex) * verts->size(), verts->data(), GL_STATIC_DRAW); }
				else
				{
					// only fast draw path
					m_bufferLockManager->waitForLockedRange(p->rbVertices.head, p->rbVertices.bufferFragment);
					int slot = p->rbVertices.head / (sizeof(Vertex));
					for(size_t v = 0; v < verts->size(); v++)
					{
						p->rbVertices.ptr[slot+v] = (*verts)[v];
					}
					p->rbVertices.oldHead = p->rbVertices.head;
					p->rbVertices.head = (p->rbVertices.head + p->rbVertices.bufferFragment) % p->rbVertices.bufferSize;
				}

				/// indices
				//glNamedBufferData(p->indexBuffer, sizeof(GLuint) * indices->size(), indices->data(), GL_STATIC_DRAW);

				/// draw commands
				m_bufferLockManager->waitForLockedRange(p->rbCommands.head, p->rbCommands.bufferFragment);
//				int deicSlot = p->rbCommands.head / sizeof(DEIC);
//				for(size_t d = 0; d < DEICs->size(); d++)
//				{
//					p->rbCommands.ptr[deicSlot+d] = (*DEICs)[d];
//				}
				int daicSlot = p->rbCommands.head / sizeof(DAIC);
				for(size_t d = 0; d < DAICs->size(); d++)
				{
					p->rbCommands.ptr[daicSlot+d] = (*DAICs)[d];
				}
				p->rbCommands.oldHead = p->rbCommands.head;
				p->rbCommands.head = (p->rbCommands.head + p->rbCommands.bufferFragment) % p->rbCommands.bufferSize;

				/// draw IDs (needed?)
				glNamedBufferData(p->drawIDBuffer, sizeof(GLuint) * p->drawIDs.size(), p->drawIDs.data(), GL_STATIC_DRAW);

				/// transforms
				if(p->pathType == DrawPathType::STATIC)  { glNamedBufferData(p->transformBuffer, sizeof(glm::mat4) * transforms->size(), transforms->data(), GL_DYNAMIC_DRAW); }
				else
				{
					// fast and long draw paths
					m_bufferLockManager->waitForLockedRange(p->rbTransforms.head, p->rbTransforms.bufferFragment);
					int slot = p->rbTransforms.head / sizeof(glm::mat4);
					for(size_t t = 0; t < transforms->size(); t++)
					{
						p->rbTransforms.ptr[slot+t] = (*transforms)[t];
					}
					p->rbTransforms.oldHead = p->rbTransforms.head;
					p->rbTransforms.head = (p->rbTransforms.head + p->rbTransforms.bufferFragment) % p->rbTransforms.bufferSize;
				}

				/// texture addresses
				glNamedBufferData(p->textureAddressBuffer, sizeof(TextureAddress) * texAddrs->size(), texAddrs->data(), GL_DYNAMIC_DRAW);

//				Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Renderer2D::drawPrep()",
//					"\n\033[1m\033[93mEND DAIC GENERATION\033[0m (total {}):\n\tvertices\t\t{}\n\tindices\t\t\t{}\n\tdrawIDs\t\t\t{}\n\ttransforms\t\t{}\n\ttxtAddrs\t\t{}\n",
//					DAICs->size(), verts->size(), indices->size(), p->drawIDs.size(), transforms->size(), texAddrs->size());
			}
			// special section to ensure at LEAST transforms are updated for a long draw path(s)
			else if(p->updated == false && p->pathType == DrawPathType::LONG && p->glyphs.size() != 0)
			{
				transforms = &p->transforms;
				transforms->clear();

				int t = 0;
				m_bufferLockManager->waitForLockedRange(p->rbTransforms.head, p->rbTransforms.bufferFragment);
				int slot = p->rbTransforms.head / sizeof(glm::mat4);
				for(auto& it : p->glyphs)
				{
					glm::mat4 transRotate = glm::rotate(identMat, glm::radians(it->angle), glm::vec3(0.0f, 0.0f, 1.0f));
					transforms->push_back(transRotate);
					transforms->back() = glm::translate(transforms->back(), it->position);
					p->rbTransforms.ptr[slot+t] = transforms->back();
					t++;
				}

				p->rbTransforms.oldHead = p->rbTransforms.head;
				p->rbTransforms.head = (p->rbTransforms.head + p->rbTransforms.bufferFragment) % p->rbTransforms.bufferSize;
			}
			//glBindVertexArray(0);	// *should* be unnecessary using glNamedBufferData
		}
	}

	/**
	 * @brief actually render the three, default draw paths (fast, long, and static)
	 */
	void Renderer2D::render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.45f, 0.5f, 0.55f, 1.0f);

		if(m_staticDrawPath.drawCommands.size() > 0) { renderDrawPath(&m_staticDrawPath, m_prog->getProgramID()); }		// static draw path (e.g. terrain, background murals, etc)
		if(m_longDrawPath.drawCommands.size() > 0) { renderDrawPath(&m_longDrawPath, m_prog->getProgramID()); }			// long draw path (e.g. characters, abilities)
		if(m_fastDrawPath.drawCommands.size() > 0) { renderDrawPath(&m_fastDrawPath, m_prog->getProgramID()); }			// fast/temporary/ephemeral draws

		// 'un-use' the GLSL program
		glUseProgram(0);
	}

	/**
	 * @brief render a passed draw path (used by render() for default draw paths )
	 *
	 * @param p draw path to render
	 * @param glProgID GLSL program ID (if applicable, will fallback to default GLSL program)
	 */
	void Renderer2D::renderDrawPath(DrawPath* p, GLuint glProgID /*= 0*/)
	{
        if(!p) { Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "Renderer2D::renderDrawPath", "nullptr passed for draw path."); }
        else if(p->drawCmdBuffer && p->drawCommands.size() <= 0) { /*Logger::getInstance().Log(Logs::VERBOSE, Logs::Drawing, "Renderer2D::renderDrawPath", "Passed draw path has no draw commands, but is using the draw command buffer.");*/ }
        else if(p->glyphs.size() <= 0) { Logger::getInstance().Log(Logs::VERBOSE, Logs::Drawing, "Renderer2D::renderDrawPath", "Passed DrawPath has no glyphs to render!"); }
        else if(p->vertexArrayObject == 0) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::renderDrawPath", "Passed draw path does *NOT* have a valid vertex array object!"); }
        else
        {
//			Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Renderer2D::renderDrawPath()", "\n\tpathType [{}],\n\tVAO [{}],\n\tVBO [{}],\n\tvertices size [{}],\n\tIBO [{}],\n\tindices size [{}],\n\tXFormBO [{}],\n\ttransforms size [{}],\n\ttxAddrBuffer [{}],\n\ttxAddr size [{}],\n\tdrawIDBuffer [{}],\n\tdrawIDs size [{}],\n\tdrawCmdBuffer [{}],\n\tdrawCommands size [{}]",
//				p->pathType, p->vertexArrayObject, p->vertexBuffer, p->vertices.size(), p->indexBuffer, p->indices.size(), p->transformBuffer, p->transforms.size(), p->textureAddressBuffer, p->textureAddresses.size(), p->drawIDBuffer, p->drawIDs.size(), p->drawCmdBuffer, p->drawCommands.size());

			int activeProgramID = 0; // currently used glsl program
			glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgramID);

			// active passed glsl program id, or enable existing if not already enabled
			if(glProgID > 0) { glUseProgram(glProgID); }
			else if(activeProgramID == 0) { glUseProgram(m_prog->getProgramID()); }

			// all clear, do it!
			glBindVertexArray(p->vertexArrayObject);

			// bind SSBOs, if applicable
			if(p->transformBuffer) { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, p->transformBuffer); }
			if(p->textureAddressBuffer) { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, p->textureAddressBuffer); }

			// finally render
//			if(p->drawCmdBuffer) { glBindBuffer(GL_DRAW_INDIRECT_BUFFER, p->drawCmdBuffer); glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, p->drawCommands.size(), 0); }
//			else { glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, p->drawCommands.data(), p->drawCommands.size(), 0); }
			//glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, p->drawCommands.data(), p->drawCommands.size(), 0);	// DEBUG MEASURE
			if(p->drawCmdBuffer)
			{
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, p->drawCmdBuffer);
                glMultiDrawArraysIndirect(GL_TRIANGLES, 0, p->drawCommands.size(), 0);
            }
			else { glMultiDrawArraysIndirect(GL_TRIANGLES, p->drawCommands.data(), p->drawCommands.size(), 0); }

			// update ring buffer(s), if applicable
			if(p->rbCommands.ptr != nullptr) { m_bufferLockManager->lockRange(p->rbCommands.oldHead, p->rbCommands.bufferFragment); }
			if(p->rbTransforms.ptr != nullptr) { m_bufferLockManager->lockRange(p->rbTransforms.oldHead, p->rbTransforms.bufferFragment); }
			if(p->rbVertices.ptr != nullptr) { m_bufferLockManager->lockRange(p->rbVertices.oldHead, p->rbVertices.bufferFragment); }

			// options specific to a "fast" draw path (if a fast draw path, glyphs are single use)
			if(p->pathType == DrawPathType::FAST) { p->clear(true); }

			// clean up
			glBindVertexArray(0);

			// change to previous glProgram
			if(activeProgramID) { glUseProgram(activeProgramID); }
			else { glUseProgram(0); }
        }
	}

	/**
	 * @brief initialize the passed draw path, setting up ring buffers, configuring path type, etc
	 *
	 * @param p draw path to initialize
	 */
	void Renderer2D::setupDrawPath(DrawPath* p)
	{
		if(!p) { Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "Renderer2D::setupDrawPath", "nullptr passed for draw path."); }
		else if(p->pathType == DrawPathType::NONE || p->pathType >= DrawPathType::INVALID) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::setupDrawPath", "Passed pathType is invalid!"); }
		else
		{
			// not sure what this actually does or would be used for...
			int vertexBindingIndex = 0, drawIDBindingIndex = 2;

			// VAO
			glGenVertexArrays(1, &p->vertexArrayObject);
			glBindVertexArray(p->vertexArrayObject);
			int vao = p->vertexArrayObject;

			// vertices
			glCreateBuffers(1, &p->vertexBuffer);
			glVertexArrayVertexBuffer(vao, vertexBindingIndex, p->vertexBuffer, 0, sizeof(Vertex));
			glEnableVertexArrayAttrib(vao, 0);
			glEnableVertexArrayAttrib(vao, 1);
			glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
			glVertexArrayAttribBinding(vao, 0, vertexBindingIndex);
			glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_TRUE, offsetof(Vertex, uv));
			glVertexArrayAttribBinding(vao, 1, vertexBindingIndex);

			if(p->pathType == DrawPathType::FAST)
			{
				glNamedBufferStorage(p->vertexBuffer, p->rbVertices.bufferSize, nullptr, m_persistentCreateFlags);
				p->rbVertices.ptr = (Vertex*)glMapNamedBufferRange(p->vertexBuffer, 0, p->rbVertices.bufferSize, m_persistentMapFlags);
				p->rbVertices.bufferFragment = p->rbVertices.bufferSize / 3;
			}

			// indices
			//glCreateBuffers(1, &p->indexBuffer);
			//glVertexArrayElementBuffer(vao, p->indexBuffer);

			// draw commands
			glCreateBuffers(1, &p->drawCmdBuffer);
			//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, p->drawCmdBuffer);
			glNamedBufferStorage(p->drawCmdBuffer, p->rbCommands.bufferSize, nullptr, m_persistentCreateFlags);
//			p->rbCommands.ptr = (DEIC*)glMapNamedBufferRange(p->drawCmdBuffer, 0, p->rbCommands.bufferSize, m_persistentMapFlags);
			p->rbCommands.ptr = (DAIC*)glMapNamedBufferRange(p->drawCmdBuffer, 0, p->rbCommands.bufferSize, m_persistentMapFlags);
			p->rbCommands.bufferFragment = p->rbCommands.bufferSize / 3;

			// draw IDs
			glCreateBuffers(1, &p->drawIDBuffer);
			glVertexArrayVertexBuffer(vao, drawIDBindingIndex, p->drawIDBuffer, 0, sizeof(GLuint));
			glEnableVertexArrayAttrib(vao, 2);
			glVertexArrayAttribIFormat(vao, 2, 1, GL_UNSIGNED_INT, 0);
			glVertexArrayAttribBinding(vao, 2, drawIDBindingIndex);
			glVertexArrayBindingDivisor(vao, drawIDBindingIndex, 1);

			// transforms
			glCreateBuffers(1, &p->transformBuffer);
			if(p->pathType == DrawPathType::LONG || p->pathType == DrawPathType::FAST)
			{
				glNamedBufferStorage(p->transformBuffer, p->rbTransforms.bufferSize, nullptr, m_persistentCreateFlags);
				p->rbTransforms.ptr = (glm::mat4*)glMapNamedBufferRange(p->transformBuffer, 0, p->rbTransforms.bufferSize, m_persistentMapFlags);
				p->rbTransforms.bufferFragment = p->rbTransforms.bufferSize / 3;
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, p->transformBuffer);
			}

			// texture addresses
			glCreateBuffers(1, &p->textureAddressBuffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, p->textureAddressBuffer);
			glBindVertexArray(0);
		}
	}

	/**
	 * @brief set the camera matrix (if a change is required out of band)
	 *
	 * @param cameraMatrix passed camera matrix
	 * @param glslProgID specify GLSL program to use
	 */
	void Renderer2D::setCameraMatrix(const glm::mat4& cameraMatrix, GLuint glslProgID)
	{
        if(glslProgID == 0) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "Renderer2D::setCameraMatrix", "Passed GLSL program ID is 0!"); }
        else
        {
			glUseProgram(glslProgID);
			glUniformMatrix4fv(m_uniformMVP, 1, GL_FALSE, &cameraMatrix[0][0]);
        }
	}

	/**
	 * @brief configure window dimensions for scaling purposes
	 *
	 * @param scrWidth screen width in pixels
	 * @param scrHeight screen height in pixels
	 */
	void Renderer2D::setWindowDimensions(float scrWidth, float scrHeight)
	{
		m_screenWidth = scrWidth;
		m_screenHeight = scrHeight;
		glViewport(0, 0, m_screenWidth, m_screenHeight);
	}

	/// Renderer2D private functions //////////////////////////////////////////

	Renderer2D::Renderer2D()
	{
		//ctor
	}

	Renderer2D::~Renderer2D()
	{
		for(auto& p : m_drawPaths)
		{
			glBindVertexArray(p->vertexArrayObject);
			for(unsigned int ch = 0; ch < p->textureAddresses.size(); ch++) { glMakeTextureHandleNonResidentARB(p->textureAddresses[ch].containerHandle); }
			glDeleteBuffers(1, &p->drawIDBuffer);
			//glDeleteBuffers(1, &p->indexBuffer);
			if(p->rbVertices.ptr) { glBindBuffer(GL_ARRAY_BUFFER, p->vertexBuffer); glUnmapBuffer(GL_ARRAY_BUFFER); glBindBuffer(GL_ARRAY_BUFFER, 0); }
			glDeleteBuffers(1, &p->vertexBuffer);
			if(p->rbTransforms.ptr) { glBindBuffer(GL_SHADER_STORAGE_BUFFER, p->transformBuffer); glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); }
			glDeleteBuffers(1, &p->transformBuffer);
			glDeleteBuffers(1, &p->textureAddressBuffer);
			if(p->rbCommands.ptr) { glBindBuffer(GL_DRAW_INDIRECT_BUFFER, p->drawCmdBuffer); glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER); glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0); }
			glBindVertexArray(0);
			glDeleteVertexArrays(1, &p->vertexArrayObject);
		}
		safeDelete(m_prog);
	}

	/**
	 * @brief create the actual glyph and supply to the passed draw path
	 *
	 * @param path draw path to receive the glyph
	 * @param tex texture address (container and slice numbers)
	 * @param position x/y/z coordinates to draw object at
	 * @param dimensions width and heigh values of the object
	 * @param textureCoordinates a.k.a UV coordinates
	 * @param angle degree of rotation (radians)
	 * @param isCentered is this position the center of the object
	 * @return Glyph* pointer to new glyph object
	 */
	Glyph* Renderer2D::createGlyph(DrawPath* path, const TextureAddress& tex, const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& textureCoordinates, float angle, bool isCentered)
	{
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Renderer2D::createGlyph", "Dimensions are [{}, {}]", dimensions.x, dimensions.y);
		std::vector<Vertex> verts;
		if(isCentered)
		{
			verts.push_back({glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(textureCoordinates.x, textureCoordinates.y)});
			verts.push_back({glm::vec3(dimensions.x, 0.0f, 0.0f), glm::vec2(textureCoordinates.x + textureCoordinates.z, textureCoordinates.y)});
			verts.push_back({glm::vec3(dimensions.x, dimensions.y, 0.0f), glm::vec2(textureCoordinates.x + textureCoordinates.z, textureCoordinates.y + textureCoordinates.w)});

			verts.push_back({glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(textureCoordinates.x, textureCoordinates.y)});
			verts.push_back({glm::vec3(dimensions.x, dimensions.y, 0.0f), glm::vec2(textureCoordinates.x + textureCoordinates.z, textureCoordinates.y + textureCoordinates.w)});
			verts.push_back({glm::vec3(0.0f, dimensions.y, 0.0f), glm::vec2(textureCoordinates.x, textureCoordinates.y + textureCoordinates.w)});
		}
		else
		{
			glm::vec2 halfDim = dimensions * 0.5f;
			verts.push_back({glm::vec3(-halfDim.x, -halfDim.y, 0.0f), glm::vec2(textureCoordinates.x, textureCoordinates.y)});
			verts.push_back({glm::vec3( halfDim.x, -halfDim.y, 0.0f), glm::vec2(textureCoordinates.x + textureCoordinates.z, textureCoordinates.y)});
			verts.push_back({glm::vec3( halfDim.x,  halfDim.y, 0.0f), glm::vec2(textureCoordinates.x + textureCoordinates.z, textureCoordinates.y + textureCoordinates.w)});

			verts.push_back({glm::vec3(-halfDim.x, -halfDim.y, 0.0f), glm::vec2(textureCoordinates.x, textureCoordinates.y)});
			verts.push_back({glm::vec3( halfDim.x,  halfDim.y, 0.0f), glm::vec2(textureCoordinates.x + textureCoordinates.z, textureCoordinates.y + textureCoordinates.w)});
			verts.push_back({glm::vec3(-halfDim.x, halfDim.y, 0.0f), glm::vec2(textureCoordinates.x, textureCoordinates.y + textureCoordinates.w)});
		}

		Glyph* newGlyph = new Glyph(); // create a glyph object
		newGlyph->textureAddress = tex;
		newGlyph->vertices = verts;
		newGlyph->position = position;
		newGlyph->angle = angle;
		path->glyphs.push_back(newGlyph); // add to draw path's vector
		path->updated = true; // tell draw path to update
		return newGlyph;
	}
}
