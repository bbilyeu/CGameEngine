#include "AudioEngine.h"
#include "srv/Logger.h"

namespace CGameEngine
{

    void SoundEffect::play(int loops)
    {
        if(Mix_PlayChannel(-1, m_chunk, loops) == -1)
        {
            if(Mix_PlayChannel(0, m_chunk, loops) == -1)
            {
               Logger::getInstance().Log(Logs::FATAL, Logs::Audio, "SoundEffect::play()", "Mix_PlayChannel error: {}", Mix_GetError());
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    void Music::play(int loops)
    {
        Mix_PlayMusic(m_music, loops);
    }

    void Music::pause()
    {
        Mix_PauseMusic();
    }

    void Music::stop()
    {
        Mix_HaltMusic();
    }

    void Music::resume()
    {
        Mix_ResumeMusic();
    }

    ///////////////////////////////////////////////////////////////////////////

    AudioEngine::AudioEngine() : m_effectMap(), m_musicMap() {}
    AudioEngine::~AudioEngine() { destroy(); }

    void AudioEngine::destroy()
    {
        if(m_isInitialized)
        {
            m_isInitialized = false;
            for(auto& it : m_effectMap) { Mix_FreeChunk(it.second); }
            for(auto& it : m_musicMap) { Mix_FreeMusic(it.second); }
            m_effectMap.clear();
            m_musicMap.clear();
            Mix_CloseAudio(); // shuts off ALL audio
            Mix_Quit();
        }
    }

    void AudioEngine::init()
    {
        // prevent duplication
        if(m_isInitialized) { Logger::getInstance().Log(Logs::FATAL, Logs::Audio, "AudioEngine::init()", "Tried to initialize AudioEngine twice!"); }

        // Parameter can be a bitwise combinations of:
        //      MIX_INIT_FAC, MIX_INIT_MOD, MIX_INIT_MP3, MIX_INIT_OGG
        if(Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == -1) /// ENSURE THIS IS CALLED AFTER INITIALIZING SDL
        {
           Logger::getInstance().Log(Logs::FATAL, Logs::Audio, "AudioEngine::init()", "Mix_Init error: {}", Mix_GetError());
        }

        // This init function handles frequency, format, and sound channels
        if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
        {
           Logger::getInstance().Log(Logs::FATAL, Logs::Audio, "AudioEngine::init()", "Mix_OpenAudio error: {}", Mix_GetError());
        }

        m_isInitialized = true;

    }

    Music AudioEngine::loadMusic(const std::string& filePath)
    {
        Music music; // Music to return

        auto it = m_musicMap.find(filePath);  // Check against map (cache)
        if(it == m_musicMap.end())
        {
            // load it
            Mix_Music* mixmusic = Mix_LoadMUS(filePath.c_str());
            if(mixmusic == nullptr)
            {
               Logger::getInstance().Log(Logs::FATAL, Logs::Audio, "AudioEngine::loadMusic()", "Mix_LoadMUS error: {}, FilePath: {}", Mix_GetError(), filePath);
            }

            // new Music, add to map
            music.m_music = mixmusic;
            m_musicMap[filePath] = mixmusic;
        }
        else { music.m_music = it->second; } // found existing Music

        // return soundeffect object data
        return music;
    }

    SoundEffect AudioEngine::loadSoundEffect(const std::string& filePath)
    {
        SoundEffect effect; // effect to return

        auto it = m_effectMap.find(filePath);  // Check against map (cache)
        if(it == m_effectMap.end())
        {
            // load it
            Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str());
            if(chunk == nullptr)
            {
               Logger::getInstance().Log(Logs::FATAL, Logs::Audio, "AudioEngine::loadSoundEffect()", "Mix_LoadWAV error: {}, FilePath: {}", Mix_GetError(), filePath);
            }

            // new effect, add to map
            effect.m_chunk = chunk;
            m_effectMap[filePath] = chunk;
        }
        else { effect.m_chunk = it->second; } // found existing effect

        // return soundeffect object data
        return effect;
    }
}
