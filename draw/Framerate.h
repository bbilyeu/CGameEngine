#ifndef FRAMERATE_H_INCLUDED
#define FRAMERATE_H_INCLUDED
#include <SDL2/SDL.h>

/// \TODO : Evaluate this becoming either an inherited Time instance or replacing it with Time entirely

namespace CGameEngine
{
    class FPSLimiter
    {
        public:
            FPSLimiter();

            void beginFrame(); // begin()
            float endFrame(); // returns FPS
            const float& getDeltaTime() const { return m_totalDeltaTime; }
            const float& getMaxDeltaTime() const { return MAX_DELTA_TIME; }
            const float& getMaxFPS() const { return m_maxFPS; }
            const int& getMaxSteps() const { return MAX_PHYSICS_STEPS; }
            const uint32_t& getTicks() const { return m_startFrameTicks; }
            void init(float targetFPS = 60.0f) { setMaxFPS(targetFPS); }
            void setMaxFPS(float targetFPS) { m_maxFPS = targetFPS; DESIRED_FRAMETIME = MS_PER_SECOND / m_maxFPS; }

        private:
            float m_fps = 0.0f;
            float m_maxFPS = 60.0f;
            float m_totalDeltaTime = 0.0f;
            uint32_t m_frameTime = 0.0f;
            uint32_t m_prevTicks = 0.0f;
            uint32_t m_startFrameTicks = 0;

            const int MAX_PHYSICS_STEPS = 6;
            const float MS_PER_SECOND = 1000.0f;
            const float MAX_DELTA_TIME = 1.0f;
            float DESIRED_FRAMETIME = MS_PER_SECOND / m_maxFPS;
    };
}


#endif // FRAMERATE_H_INCLUDED
