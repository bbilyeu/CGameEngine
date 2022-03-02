#include "gui/GUI.h"
#include "Window.h"
#include "srv/InputManager.h"
#include <cstring>
#include "gui/GUIRenderer.h"

namespace CGameEngine
{
    /// GUI public functions below ////////////////////////////////////////////

    GUI::GUI() : m_frames(), m_radiogroups() { } // empty

    GUI::~GUI()
    {
		dispose();
    }

    void GUI::dispose()
    {
		m_isRunning = false;
		//if(e_renderer) { e_renderer->dispose(); } // stop renderer to prevent segfault

        for(auto& i : m_frames) { safeDelete(i.second); }
        m_frames.clear();

        for(auto& i : m_radiogroups) { safeDelete(i); }
        m_radiogroups.clear();
        safeDelete(m_contextRetVal);
        safeDelete(m_toolTip);
        safeDelete(m_contextMenu);
        safeDelete(m_cursorBlinkTimer);
        safeDelete(m_config);

        // clean up externals
        safeDelete(e_renderer);
        safeDelete(e_hover);
        safeDelete(e_dropdownPopout);
        e_gui = nullptr;
    }

    void GUI::init(Window* window, bool drawDebug /*= false*/, std::string font /*= ""*/, char fontSize /*= 13*/)
    {
		if(!window) { Logger::getInstance().Log(Logs::FATAL, Logs::GUI, "GUI::init()", "Passed window ptr was null!"); }
        CGameEngine::TextureManager::getInstance().getTexture("Assets/circle.png"); // dummy initialization
        m_window = window;
        m_drawDebug = drawDebug;
        e_renderer = new GUIRenderer(this, m_drawDebug, font, fontSize);
        e_screenHeight = m_window->getScreenHeight();
        e_screenWidth = m_window->getScreenWidth();
        e_quadHeight = e_screenHeight / 2.0f;
        e_quadWidth = e_screenWidth / 2.0f;
        Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "GUI::init()", "Window dimensions ({} x {})", e_screenWidth, e_screenHeight);
        m_guiCamera.init(e_screenWidth, e_screenHeight);
        //m_guiCamera.setScale(1.0f);
        m_maxToolTipWidth = e_screenWidth * 0.2f;
        m_inputManager = &InputManager::getInstance();
        m_cursorBlinkTimer = new Timer("cursorBlink", CURSOR_BLINK, TimeUnits::Milliseconds, true);
        m_isRunning = true;
        SDL_StopTextInput();

        /// \TODO: Is this dummy init even needed with unordered maps?
        // dummy init
        //if(m_radiogroups.size() == 0) { m_radiogroups.push_back(new RadioGroup("nullRG")); }
        //if(m_frames.size() == 0) { Frame* f = addFrame("null", glm::vec4(0.0f)); }

        safeDelete(m_toolTip);
        m_toolTip = new ToolTip(TextureManager::getInstance().getTexture("Assets/Dialog.png"), glm::vec4(0.0f), COLOR_WHITE, PERMANENT_GUI_ELEMENT);
        m_toolTip->setHoverDelay(HOVER_DELAY);
        if(!m_contextRetVal) { m_contextRetVal = new int(-1); }
		e_gui = this;
    }

    /*
		This function loads in JSON data to automatically build a UI.
		It assumes the first name(s) are frames and will assign the Frame's str name to this.
    */
    /// \TODO: Cleanup and optimize
    bool GUI::buildUI(std::string filePath)
    {
		/// \TODO: Tear down UI if new file is loaded
		if(m_config) { dispose(); safeDelete(m_config); }

		// parse config file
		m_config = new CGameEngine::ConfigReader(filePath);

		// get top level members (frames) and build them
		std::string location = "/frames";
		std::string currentLocation = "";
		std::vector<std::string> frames = m_config->getMembers(location);
		glm::vec4 sizePos = glm::vec4(0.0f);
		std::vector<float> posVec;
		for(int t = 0; t < frames.size(); t++)
		{
			Logger::getInstance().Log(Logs::DEBUG, "GUI::buildUI()", "frames[{}] = '{}'", t, frames[t]);
			currentLocation = location + "/" + frames[t]; 											// frame by name (e.g. menu, editor, etc) ("/frames/someFrame")
			sizePos = glm::vec4(0.0f);																// reset x/y/w/h vector
			posVec = m_config->getArrayValue<float>(std::string(currentLocation + "/position"));	// pull position/size data from json ("/frames/someFrame/position")
			for(int sp = 0; sp < posVec.size(); sp++) { sizePos[sp] = posVec[sp]; }					// move queue into glm::vec4

            Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "GUI::buildUI()", "\033[1mcurrentLocation = '{}'\033[0m", currentLocation);
			// if successful, create frame and begin creating elements associated to said frame
			if(true)
			{
				Frame* f = addFrame(currentLocation, sizePos);
				if(!f || f == nullptr)
				{
					Logger::getInstance().Log(Logs::CRIT, Logs::GUI, "GUI::buildUI()", "Failed to create Frame! Moving to next Frame.'");
					continue; // skip to next iteration
				}
				std::vector<std::string> frameMembers = m_config->getMembers(currentLocation);
				for(int fm = 0; fm < frameMembers.size(); fm++)
				{
					Logger::getInstance().Log(Logs::DEBUG, "GUI::buildUI()", "Reading frameMembers[{}] = '{}'", fm, frameMembers[fm]);
					std::string currValue = frameMembers[fm];
					if(currValue == "setMoveable")
					{
						bool val;
						if(m_config->getValue(std::string(currentLocation + "/setMoveable"), val)) // Approximately: "/frames/someFrame/setMoveable"
						{
							Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "GUI::buildUI()", "setMoveable = '{}'", val);
							f->setMoveable(val);
						}
					}
					else if(currValue == "setTexture")
					{
						std::string val;
						if(m_config->getValue(std::string(currentLocation + "/setTexture"), val)) // Approximately: "/frames/someFrame/setTexture"
						{
							Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "GUI::buildUI()", "setTexture = '{}'", val);
							f->setTexture(val);
						}
					}
					else if(currValue == "elements" || currValue == "position" ) { } // ignore as they are handled above (position) and below (elements)
					else { Logger::getInstance().Log(Logs::WARN, Logs::GUI, "GUI::buildUI()", "frameMembers[{}] '{}' has no defined case statement. Possible typo?", fm, frameMembers[fm]); }
				}

				// check for actual items on the frame (e.g. drop downs, buttons, etc)
				// 	Note: It absolutely should have something, else it is an empty frame which is useless.
				currentLocation = currentLocation + "/elements"; // Approximately: "/frames/someFrame/elements"
				Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "GUI::buildUI()", "\033[1mcurrentLocation = '{}'\033[0m", currentLocation);
				if(true)
				{
					std::vector<std::string> frameElements = m_config->getMembers(currentLocation); // dropdowns, scrollboxes, etc
					std::string frameGroupLocation;
					for(int fe = 0; fe < frameElements.size(); fe++)
					{
						frameGroupLocation = currentLocation + "/" + frameElements[fe]; // e.g. "/frames/someFrame/elements/dropdowns"
						Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "GUI::buildUI()", "\033[1mcurrentLocation = '{}'\033[0m", currentLocation);
						Logger::getInstance().Log(Logs::DEBUG, "GUI::buildUI()", "frameElements[{}] = '{}'", fe, frameElements[fe]);
						std::vector<std::string> frameElementObjects = m_config->getMembers(frameGroupLocation); // items of the current element (e.g. dropdown1, dropdown2, etc)
						GUIObject* obj = nullptr;
						for(int feo = 0; feo < frameElementObjects.size(); feo++)
						{
							bool hasTexture = false;
							// reset sizePos, wipe posVec, then fill them
							sizePos = glm::vec4(0.0f);
							obj = nullptr;
							posVec.clear();
							posVec = m_config->getArrayValue<float>(std::string(frameGroupLocation + "/" + frameElementObjects[feo] + "/position")); // e.g. "/frames/someFrame/elements/dropdowns/someDropdown/position"
							for(int sp = 0; sp < posVec.size(); sp++) { sizePos[sp] = posVec[sp]; }

							if(frameElements[fe] == "dropdowns")
							{
                                obj = f->addDropdown(frameElementObjects[feo], sizePos);

                                // ensure we created Dropdown before populating
                                if(obj)
                                {
									std::vector<std::string> ddElements = m_config->getArrayValue(std::string(frameGroupLocation + "/" + frameElementObjects[feo] + "/setElements")); // e.g. "/frames/someFrame/elements/dropdowns/someDropdown/setElements"
									if(!ddElements.empty()) { static_cast<Dropdown*>(obj)->addElements(ddElements); }
                                }
							}
							else if(frameElements[fe] == "pushbuttons")
							{
								obj = f->addPushButton(frameElementObjects[feo], sizePos);
							}
							else if(frameElements[fe] == "radiogroups")
							{
								RadioGroup* rg = new RadioGroup(frameElementObjects[feo]);
								std::vector<std::string> rbInRG = m_config->getMembers(std::string(frameGroupLocation + "/" + frameElementObjects[feo])); // e.g. "/frames/someFrame/elements/radiogroups/rgMode"
								std::string rbLocation;
								for(int rbi = 0; rbi < rbInRG.size(); rbi ++)
								{
									rbLocation = frameGroupLocation + "/" + frameElementObjects[feo] + "/" + rbInRG[rbi]; // e.g. "/frames/someFrame/elements/radiogroups/rgMode/rbSelect"
									posVec = m_config->getArrayValue<float>(std::string(rbLocation + "/position"));
									for(int sp = 0; sp < posVec.size(); sp++) { sizePos[sp] = posVec[sp]; }
									RadioButton* rb = f->addRadioButton(rbLocation, sizePos);

									if(rb)
									{
										bool hasRBTexture = false;
										rb->joinRadioGroup(rg);
										std::vector<std::string> rbElements = m_config->getMembers(rbLocation);
										for(int rbe = 0; rbe < rbElements.size(); rbe++)
										{
											if(rbElements[rbe] == "setTicked")
											{
												bool val;
												if(m_config->getValue(std::string(rbLocation + "/" + "setTicked"), val)) { rb->setTicked(val); }
											}
											else if(rbElements[rbe] == "setTickedTexture")
											{
												std::string val;
												if(m_config->getValue(std::string(rbLocation + "/" + "setTickedTexture"), val)) { rb->setTickedTexture(val); }
											}
											else if(rbElements[rbe] == "setUntickedTexture")
											{
												std::string val;
												if(m_config->getValue(std::string(rbLocation + "/" + "setUntickedTexture"), val)) { rb->setUntickedTexture(val); }
											}
											else if(rbElements[rbe] == "addLabel")
											{
												std::vector<std::string> vec = m_config->getArrayValue(std::string(rbLocation + "/" + "addLabel"));
												if(vec.size() == 2) // ["Label Text", "Center"]
												{
													// determine positioning
													int position = GUIPositions::NONE;
													if(vec[1] == "Left") { position = GUIPositions::Left; }
													else if(vec[1] == "Right") { position = GUIPositions::Right; }
													else if(vec[1] == "Top") { position = GUIPositions::Top; }
													else if(vec[1] == "Bottom") { position = GUIPositions::Bottom; }
													else if(vec[1] == "TopLeft") { position = GUIPositions::TopLeft; }
													else if(vec[1] == "TopRight") { position = GUIPositions::TopRight; }
													else if(vec[1] == "BottomLeft") { position = GUIPositions::BottomLeft; }
													else if(vec[1] == "BottomRight") { position = GUIPositions::BottomRight; }

													// add label
													rb->addLabel(vec[0], position);
												}
											}
										}

										// ensure each RB has a texture
										if(!hasRBTexture && m_fallbackDefault) { rb->defaultTextures(); }
									}
								}
							}
							else if(frameElements[fe] == "scrollboxes")
							{
								obj = f->addScrollBox(frameElementObjects[feo], sizePos);
								if(obj)
								{
									ScrollBox* sbox = static_cast<ScrollBox*>(obj);
									std::vector<std::string> sbElements = m_config->getMembers(std::string(frameGroupLocation + "/" + frameElementObjects[feo]));
									std::string sbLocation;
									bool hasUpTexture = false, hasDownTexture = false;
									for(int sbe = 0; sbe < sbElements.size(); sbe++)
									{
										sbLocation = frameGroupLocation + "/" + frameElementObjects[feo] + "/" + sbElements[sbe];
										if(sbElements[sbe] == "setNoButtons")
										{
											bool val;
											if(m_config->getValue(sbLocation, val))
											{
												sbox->setNoButtons(val);
												if(val)
												{
													hasDownTexture = true;
													hasUpTexture = true;
												}
											}
										}
										if(sbElements[sbe] == "setDownTexture")
										{
											std::string txt;
											if(m_config->getValue(sbLocation, txt)) { sbox->setDownTexture(txt); hasDownTexture = true; }
										}
										else if(sbElements[sbe] == "setUpTexture")
										{
											std::string txt;
											if(m_config->getValue(sbLocation, txt)) { sbox->setUpTexture(txt); hasUpTexture = true; }
										}
										else if(sbElements[sbe] == "setPrecision")
										{
											int val;
											if(m_config->getValue(sbLocation, val)) { sbox->setPrecision(static_cast<uint8_t>(val)); }
										}
										else if(sbElements[sbe] == "setRange")
                                        {
											std::vector<int> val;
											val = m_config->getArrayValue<int>(sbLocation);
											if(val.size() == 2) { sbox->setRange(val[0], val[1]); }
                                        }
										else if(sbElements[sbe] == "setRolloverEnabled")
										{
											bool val;
											if(m_config->getValue(sbLocation, val)) { sbox->setRolloverEnabled(val); }
										}
										else if(sbElements[sbe] == "setStepValue")
										{
											float val = 0.0f;
											if(m_config->getValue(sbLocation , val)) { sbox->setStepValue(val); }
										}
									}

									// default texture loading, if enabled
									if(!hasDownTexture || !hasUpTexture) { sbox->defaultTextures(); }
								}
							}
							else if(frameElements[fe] == "sliders")
							{
								obj = f->addSlider(frameElementObjects[feo], sizePos);
								if(obj)
								{
									Slider* slider = static_cast<Slider*>(obj);
									std::vector<std::string> slElements = m_config->getMembers(std::string(frameGroupLocation + "/" + frameElementObjects[feo]));
									std::string slLocation;
									for(int sli = 0; sli < slElements.size(); sli++)
									{
                                        slLocation = frameGroupLocation + "/" + frameElementObjects[feo] + "/" + slElements[sli];
                                        if(slElements[sli] == "setFillSlider")
                                        {
											bool val;
											if(m_config->getValue(slLocation, val)) { slider->setFillSlider(val); }
                                        }
                                        else if(slElements[sli] == "setRange")
                                        {
											std::vector<int> val;
											val = m_config->getArrayValue<int>(slLocation);
											if(val.size() == 2) { slider->setRange(val[0], val[1]); }
                                        }
                                        else if(slElements[sli] == "setSliderTickPct")
                                        {
											std::vector<float> val;
											val = m_config->getArrayValue<float>(slLocation);
											if(val.size() == 2) { slider->setSliderTickPct(glm::vec2(val[0], val[1])); }
                                        }
                                        else if(slElements[sli] == "setUsableBoundsPct")
                                        {
											std::vector<float> val;
											val = m_config->getArrayValue<float>(slLocation);
											if(val.size() == 2) { slider->setUsableBoundsPct(glm::vec2(val[0], val[1])); }
                                        }
                                        else if(slElements[sli] == "setXBounds")
                                        {
											std::vector<float> val;
											val = m_config->getArrayValue<float>(slLocation);
											if(val.size() == 2) { slider->setXBounds(glm::vec2(val[0], val[1])); }
                                        }
									}
								}
							}
							else if(frameElements[fe] == "textobjects")
							{
								std::string defText = "";
								m_config->getValue(std::string(frameGroupLocation + "/" + frameElementObjects[feo] + "/defaultText"), defText);
								obj = f->addTextObject(frameElementObjects[feo], defText, sizePos);
							}
							else if(frameElements[fe] == "tickboxes")
							{
								obj = f->addTickBox(frameElementObjects[feo], sizePos);
								if(obj)
								{
									TickBox* tickbox = static_cast<TickBox*>(obj);
									std::vector<std::string> tbElements = m_config->getMembers(std::string(frameGroupLocation + "/" + frameElementObjects[feo]));
									std::string tbLocation;
									for(int tbi = 0; tbi < tbElements.size(); tbi++)
									{
										tbLocation = frameGroupLocation + "/" + frameElementObjects[feo] + "/" + tbElements[tbi];
										if(tbElements[tbi] == "setTicked")
										{
											bool val;
											if(m_config->getValue(tbLocation, val)) { tickbox->setTicked(val); }
										}
										else if(tbElements[tbi] == "setTickedTexture")
										{
											std::string val;
											if(m_config->getValue(tbLocation, val)) { tickbox->setTickedTexture(val); }
										}
										else if(tbElements[tbi] == "setUntickedTexture")
										{
											std::string val;
											if(m_config->getValue(tbLocation, val)) { tickbox->setUntickedTexture(val); }
										}
									}
								}
							}

							/// Shared functions from GUIObject
							// process shared functions, inherited from GUIObject (e.g. addLabel, setTextBounds, etc)
							if(obj && obj != nullptr)
							{
								std::vector<std::string> feoMembers = m_config->getMembers(std::string(frameGroupLocation + "/" + frameElementObjects[feo])); // e.g. "/frames/someFrame/elements/dropdowns/someDropdown"
								std::string deepLocation = frameGroupLocation + "/" + frameElementObjects[feo];
                                for(int feom = 0; feom < feoMembers.size(); feom++)
                                {
									std::string retVal = "";
									std::string tmpLoc = deepLocation + "/" + feoMembers[feom]; // e.g. "/frames/someFrame/elements/dropdowns/someDropdown/addLabel"

									if(feoMembers[feom] == "addIcon")
									{
										// do something
									}
									else if(feoMembers[feom] == "addLabel")
									{
										if(m_config->isArray(tmpLoc))
										{
											std::vector<std::string> vec = m_config->getArrayValue(tmpLoc);
											if(vec.size() == 2) // ["Label Text", "Center"]
											{
												// determine positioning
												int position = GUIPositions::NONE;
												if(vec[1] == "Left") { position = GUIPositions::Left; }
												else if(vec[1] == "Right") { position = GUIPositions::Right; }
												else if(vec[1] == "Top") { position = GUIPositions::Top; }
												else if(vec[1] == "Bottom") { position = GUIPositions::Bottom; }
												else if(vec[1] == "TopLeft") { position = GUIPositions::TopLeft; }
												else if(vec[1] == "TopRight") { position = GUIPositions::TopRight; }
												else if(vec[1] == "BottomLeft") { position = GUIPositions::BottomLeft; }
												else if(vec[1] == "BottomRight") { position = GUIPositions::BottomRight; }

												// add label
												obj->addLabel(vec[0], position);
											}
										}
										else if(m_config->getValue(tmpLoc, retVal)) { obj->addLabel(retVal); }
									}
									else if(feoMembers[feom] == "setColor")
									{
										glm::ivec4 c;
										std::vector<int> colors = m_config->getArrayValue<int>(tmpLoc);
										for(int ic = 0; ic < colors.size(); ic++) { c[ic] = colors[ic]; }
										obj->setColor(c);
									}
									else if(feoMembers[feom] == "setSubTextScaling")
									{
										float scaling = 0.0f;
										if(m_config->getValue(tmpLoc, scaling)) { obj->setSubTextScaling(scaling); }
									}
									else if(feoMembers[feom] == "setText")
									{
										std::string txt;
										if(m_config->getValue(tmpLoc, txt)) { obj->setText(txt); }
									}
									else if(feoMembers[feom] == "setTextBounds")
									{
										std::vector<float> bounds = m_config->getArrayValue<float>(tmpLoc);
										if(bounds.size() == 2) { obj->setTextBounds(bounds[0], bounds[1]); }
									}
									else if(feoMembers[feom] == "setTextJustification")
									{
										std::string just;
										if(m_config->getValue(tmpLoc, just))
										{
											int txtJustification = Justification::NONE;
											if(just == "Left") { txtJustification = Justification::Left; }
											else if(just == "Right") { txtJustification = Justification::Right; }
											else if(just == "Center") { txtJustification = Justification::Center; }
											obj->setTextJustification(txtJustification);
										}
									}
									else if(feoMembers[feom] == "setTextScaling")
									{
										float scaling = 0.0f;
										if(m_config->getValue(tmpLoc, scaling)) { obj->setTextScaling(scaling); }
									}
									else if(feoMembers[feom] == "setTexture")
									{
										std::string txt;
										if(m_config->getValue(tmpLoc, txt)) { obj->setTexture(txt); hasTexture = true; }
									}
									else if(feoMembers[feom] == "setToolTipMessage")
									{
										if(m_config->getValue(tmpLoc, retVal)) { obj->setToolTipMessage(retVal); }
									}
									else if(feoMembers[feom] == "setVisible")
									{
										bool setvis;
										if(m_config->getValue(tmpLoc, setvis)) { obj->setVisible(setvis); }
									}
                                }

                                // ensure textures are loaded, even if the default ones.
                                if(!hasTexture && m_fallbackDefault) { obj->defaultTextures(); }
							}
						}
					}
				}
			}
		}

		// if we didn't fail by now, it's all good
		return true;
    }

    Frame* GUI::addFrame(std::string name /*= "defaultFrame"*/, glm::vec4 destRect /*= glm::vec4(0.0f)*/)
    {
        if(destRect == glm::vec4(0.0f))
        {
			Logger::getInstance().Log(Logs::WARN, Logs::GUI, "GUI::addFrame()", "Frame cannot use blank position and size.");
			return nullptr;
		}

        // convert to screen coordinates
//        destRect.x *= e_screenWidth / 2.0f;
//        destRect.y *= e_screenHeight / 2.0f;
//        destRect.z *= e_screenWidth;
//        destRect.w *= e_screenHeight;

        Frame* f = new Frame(this, name, destRect);
        if(!f) { Logger::getInstance().Log(Logs::FATAL, Logs::GUI, "GUI::addFrame()", "Failed to create frame!"); }
        Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "GUI::addFrame()", "frame mem addr [{}]", (void*)f);
        m_frames.insert(std::make_pair(name, f));
        return (m_frames.at(name)); // return pointer to new frame
    }

    GUIObject* GUI::getObject(std::string name)
    {
		GUIObject* retVal = nullptr;
		for(auto& f : m_frames)
		{
			retVal = f.second->getObject(name);
			if(retVal && retVal != nullptr) { return retVal; }
		}
		Logger::getInstance().Log(Logs::WARN, Logs::GUI, "GUI::getObject()", "[{}] object not found!", name);
		return nullptr;
    }

    void GUI::setDebug(bool val /*= true*/)
    {
		m_drawDebug = val;
		e_renderer->setDebug(val);
	}

    RadioGroup* GUI::addRadioGroup()
    {
        // add new radio group
        m_radiogroups.push_back(new RadioGroup(m_radiogroups.size()));
        return m_radiogroups.back();
    }

    const glm::vec2 GUI::getTextMeasurements(std::string str, float scaling) { return e_guiFont->measure(str, scaling); }

    void GUI::calcTextSize(glm::vec4& passedVec, std::string text, float scaling /*= 1.0f*/)
    {
        // get approximate size of tooltip based on length/size of text
        float pixelSize = static_cast<float>(e_guiFont->fontSize);
        float fontHeight = static_cast<float>(e_guiFont->fontHeight);
        glm::vec2 fontMeasurements = getTextMeasurements(text, scaling);

        passedVec.z = fontMeasurements.x * 1.2f;
        passedVec.w = fontMeasurements.y * 1.2f;
    }

    void GUI::draw()
    {
        //if(m_isRunning) { e_renderer->renderGUI(m_guiCamera.getCameraMatrix()); }
        if(m_isRunning) { e_renderer->renderGUI(m_guiCamera.getOrtho()); }
    }

    bool GUI::onSDLEvent(SDL_Event& evnt)
    {
		if(!m_isRunning) { return false; }

        bool retVal = false;

        // generic pointers
        bool stillHasFocus = false;
        GUIObject* obj = nullptr;

        switch (evnt.type)
        {
            case SDL_QUIT:
            {
                //exitGame();
                m_isRunning = false;
				Logger::getInstance().Log(Logs::INFO, Logs::GUI, "GUI::onSDLEvent()", "GUI::onSDLEvent is calling SDL_QUIT");
                break;
			}
            case SDL_MOUSEMOTION:
            {
                m_prevMouseVec = e_mouseVec;
                e_mouseVec = glm::vec2(static_cast<float>(evnt.motion.x), static_cast<float>(e_screenHeight - evnt.motion.y));
                obj = guiCollision(GUIMouse::MouseMove, stillHasFocus);
                toolTipLogic();
                //Logger::getInstance().Log(Logs::INFO, Logs::GUI, "GUI::onSDLEvent()", "mouseVec ({}, {})", e_mouseVec.x, e_mouseVec.y);
                break;
            }
            case SDL_KEYDOWN:
            {
                // lazy catch
                if(m_lastTouchedObject == nullptr) { break; }
                else if(!SDL_IsTextInputActive()) { break; }

                SDL_Keycode key = evnt.key.keysym.sym;
                //Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "GUI::onSDLEvent()", "SDL_KEYDOWN: {}", key);

                if(key == SDLK_BACKSPACE)
                {
                    if(m_inputString.length() > 0)
                    {
                        m_inputString = m_inputString.substr(0, m_inputString.length()-1);
                        m_lastTouchedObject->setText(m_inputString);
                    }
                    else
                    {
                        m_inputString = " ";
                        m_lastTouchedObject->setText(m_inputString);
                    }
                }
                else if(key == SDLK_ESCAPE)
                {
                    Logger::getInstance().Log(Logs::DEBUG, "GUI::onSDLEvent()", "GUI got SDLK_ESCAPE.");
                    m_lastTouchedObject->resetText();
                    touchObject(m_lastTouchedObject, true);
                }
                else if(key == SDLK_RETURN)
                {
                    m_lastTouchedObject->trySetValue(m_inputString);
                    touchObject(m_lastTouchedObject, true);
                }
                else if(key == SDLK_c && SDL_GetModState() & KMOD_CTRL)
                {
                    SDL_SetClipboardText(m_inputString.c_str());
                }
                else if(key == SDLK_v && SDL_GetModState() & KMOD_CTRL)
                {
                    m_inputString = SDL_GetClipboardText();
                    m_lastTouchedObject->setText(m_inputString);
                }

                if(m_lastTouchedObject) { retVal = true; }
                break;
            }
            case SDL_TEXTINPUT:
            {
                // lazy catch
                if(m_lastTouchedObject == nullptr) { break; }
                else if(!SDL_IsTextInputActive()) { break; }

                if( !((evnt.text.text[ 0 ] == 'c' || evnt.text.text[ 0 ] == 'C') && (evnt.text.text[ 0 ] == 'v' || evnt.text.text[ 0 ] == 'V') && SDL_GetModState() & KMOD_CTRL) )
                {
					Logger::getInstance().Log(Logs::DEBUG, "GUI::onSDLEvent()", "SDL_TEXTINPUT: Key press: {}", evnt.text.text);
                    if(m_inputString == " ") { m_inputString = evnt.text.text; }
                    else if(m_inputString.back() == '_') { m_inputString = m_inputString.substr(0, m_inputString.length()-1) + evnt.text.text; }
                    else { m_inputString += evnt.text.text; }
					Logger::getInstance().Log(Logs::DEBUG, "GUI::onSDLEvent()", "SDL_TEXTINPUT: InputString: {}", m_inputString);
                    m_lastTouchedObject->setText(m_inputString);
                }

                retVal = true;
                break;
            }
            case SDL_KEYUP:
                //inputManager->releaseKey(evnt.key.keysym.sym);
                break;
            case SDL_MOUSEBUTTONDOWN:
            {
                if(evnt.button.button == SDL_BUTTON_LEFT) { obj = guiCollision(GUIMouse::LeftClick, stillHasFocus); }
                else if(evnt.button.button == SDL_BUTTON_RIGHT) { obj = guiCollision(GUIMouse::RightClick, stillHasFocus); }
                if(obj)
                {
                   Logger::getInstance().Log(Logs::DEBUG, "GUI::onSDLEvent()", "SDL_MOUSEBUTTONDOWN: Returned obj [{}]", static_cast<void*>(obj));
                    touchObject(obj);
                }
                break;
            }
            case SDL_MOUSEBUTTONUP:
                //inputManager->releaseKey(evnt.button.button);
                break;
            case SDL_MOUSEWHEEL:
            {
                int scrollAction = GUIMouse::MWheelUp;
                if(evnt.wheel.y < 0) { scrollAction = GUIMouse::MWheelDown; }
                m_scrollVec = glm::vec2(static_cast<float>(evnt.wheel.x), static_cast<float>(evnt.wheel.y));
                obj = guiCollision(scrollAction, stillHasFocus);
                break;
            }
        }

        if(obj || stillHasFocus) { retVal = true; }
        return retVal;
    }

    void GUI::update(float deltaTime /*= 0.0f*/)
    {
		if(!m_isRunning) { return; }

        // update frames and their elements
        for(auto& i : m_frames) { i.second->update(deltaTime); }

        // tooltip
        if(m_toolTip)
        {
            bool ttDestroy = m_toolTip->update(deltaTime);
            if(ttDestroy) { m_toolTip->destroy(); }
        }

        // context menu
        if(m_contextMenu)
        {
            bool cmDestroy = m_contextMenu->update(deltaTime);
            if(cmDestroy) { safeDelete(m_contextMenu); }
        }

        // dropdown popout
        if(e_dropdownPopout) { e_dropdownPopout->update(deltaTime); }


        // blinking cursor
        if(SDL_IsTextInputActive())
        {
            if(m_lastTouchedObject && m_cursorBlinkTimer->isExpired())
            {
                m_cursorBlinkTimer->restart();
                /// \TODO: Set cursor invis?
                if(m_lastTouchedObject->hasCursor()) { m_lastTouchedObject->setCursor(false); }
                else { m_lastTouchedObject->setCursor(true); }
            }
        }

        // update camera
        m_guiCamera.update();
    }

    ContextMenu* GUI::addContextMenu(glm::vec2 dimensions, std::vector<std::string> elements, int* retVal)
    {
        safeDelete(m_contextMenu);
        glm::vec2 sc = convertV2ScrToWorld(e_mouseVec, e_screenWidth, e_screenHeight);
        m_contextMenu = new ContextMenu(glm::vec4(sc.x, sc.y, dimensions[0], dimensions[1]), elements, retVal);
        return m_contextMenu;
    }

    /// GUI Private functions below ///////////////////////////////////////////

    /// \TODO: Fix dragging Frame when an element is under cursor (lock element while mouse button down?)
    GUIObject* GUI::guiCollision(int action, bool& stillHasFocus)
    {
		bool isKeyHeld = false;
		if(m_inputManager) { isKeyHeld = m_inputManager->isKeyDown(SDL_BUTTON_LEFT); }

		// check frames and their contents
		if(m_lastFrame && isInside(e_mouseVec, m_lastFrame->getScreenDestRect()))
		{
			e_hover = m_lastFrame->getCollisionObject(e_mouseVec, m_prevMouseVec, action, isKeyHeld);
			return e_hover;
		}
		else
		{
			// if dropdownPopout exists and has a collision, handle it first
			if(e_dropdownPopout && isInside(e_mouseVec, e_dropdownPopout->getScreenDestRect()))
			{
				bool destroy = e_dropdownPopout->hasCollision(e_mouseVec, action);
				if(destroy) { safeDelete(e_dropdownPopout); stillHasFocus = true; return nullptr; }
				else { stillHasFocus = true; e_hover = (GUIObject*)e_dropdownPopout->getParent(); return e_hover;}
			}

			// if content menu exists, test for it
			if(m_contextMenu && isInside(e_mouseVec, m_contextMenu->getScreenDestRect()))
			{
				bool destroy = m_contextMenu->hasCollision(e_mouseVec, action);
				if(destroy) { safeDelete(m_contextMenu); stillHasFocus = true; return nullptr; }
				else { stillHasFocus = true; e_hover = (GUIObject*)m_contextMenu; return e_hover; }
			}

			for(auto& i : m_frames)
			{
				// if inside the frame
				if(isInside(e_mouseVec, i.second->getScreenDestRect()))
				{
					stillHasFocus = true; // store frame

					// return the object being collided with (i.e. button, slider, etc)
					e_hover = i.second->getCollisionObject(e_mouseVec, m_prevMouseVec, action, isKeyHeld);
					m_lastFrame = i.second;
					return e_hover;
				}
			}
		}

		// catch to ensure large, fast movements of frames are accounted for
		if(isKeyHeld && m_lastFrame)
		{
			m_lastFrame->getCollisionObject(e_mouseVec, m_prevMouseVec, action, isKeyHeld);
		}
		else { m_lastFrame = nullptr; }

        if(!stillHasFocus) { e_hover = nullptr; }

        return nullptr;
    }

    int GUI::getPixelSize()
    {
        // 10pt is 13px
        // 12pt is 16px
        // 14pt is 19px
        // 16pt is 22px
        int pixelSize = 0;
        switch (e_guiFont->fontSize)
        {
            case 12:
                pixelSize = 16;
                break;
            case 14:
                pixelSize = 19;
                break;
            case 16:
                pixelSize = 22;
                break;
            default:
                pixelSize = 13;
                break;
        }

        return pixelSize;
    }

    void GUI::toolTipLogic()
    {
        if(m_toolTip->isActive())
        {
            if(e_hover == nullptr) { m_toolTip->destroy(); } // lost hover, hide tooltip
            else { m_toolTip->setHoverObject(); } // new object, just shift
        }
        else
        {
            // new object, start process
            if(e_hover) { m_toolTip->setHoverObject(true); }
        }
    }

    void GUI::touchObject(GUIObject* obj, bool done /*= false*/)
    {
        if(obj == nullptr && !done) { return; } // lazy catch

        if(!done) // if not done editing
        {
            // cleanup previous item
            if(m_lastTouchedObject)
            {
                m_lastTouchedObject->setCursor(false);
                m_inputString = "";
                m_lastTouchedObject = nullptr;
                m_cursorBlinkTimer->setExpired();
            }

            // get text
            if(obj->hasText()) { m_inputString = obj->getText(); }
            else { m_inputString = ""; }

            // actual assignment
            m_lastTouchedObject = obj;

            // check against object's m_isInEditMode to start input
            if(obj->isInEditMode())
            {
                // start blink counter
                m_cursorBlinkTimer->restart();

                // start accepting input
                SDL_StartTextInput();
            }
            else { SDL_StopTextInput(); }

        }
        else // if done editing
        {
           Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "GUI::onSDLEvent()", "Releasing Object obj [{}], done [{}], or type [{}]", (void*)obj, done, obj->getType());
            obj->setCursor(false);
            m_inputString = "";
            m_lastTouchedObject = nullptr;
            m_cursorBlinkTimer->setExpired();
            SDL_StopTextInput();
        }
    }
}
