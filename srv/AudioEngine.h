#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <SDL2/SDL_mixer.h>
#include <string>
#include <map>

// NOTE: SDL_mixer does mixed_chunk (sound effect) and mixed_music (music)

namespace CGameEngine
{
    class SoundEffect
    {
        public:
            friend class AudioEngine;
            void play(int loops = 0); // loops here indicates additional loops (i.e. 1 + loops)

        private:
            Mix_Chunk* m_chunk = nullptr;
    };

    class Music
    {
        public:
            friend class AudioEngine;

            // loops here indicates number of times to play (i.e. 0 + loops)
            void play(int loops = -1); // loops of -1 is unlimited

            static void pause();
            static void stop();
            static void resume();

        private:
            Mix_Music* m_music = nullptr;
    };

    class AudioEngine
    {
        public:
            AudioEngine();
            ~AudioEngine();

            void destroy(); // release all resources, stop all audio, etc
            void init();

            Music loadMusic(const std::string& filePath);
            SoundEffect loadSoundEffect(const std::string& filePath);

        private:
            bool m_isInitialized = false;
            std::map<std::string, Mix_Chunk*> m_effectMap;
            std::map<std::string, Mix_Music*> m_musicMap;
    };
}

#endif // AUDIOENGINE_H
