#include "draw/Framerate.h"

namespace CGameEngine
{
    FPSLimiter::FPSLimiter()
    {
        m_prevTicks = SDL_GetTicks(); // set to 0
    }

    /**
     * @brief start of a frame, shift/reset values
     */
    void FPSLimiter::beginFrame()
    {
        m_startFrameTicks = SDL_GetTicks(); // get current ticks
        m_frameTime = m_startFrameTicks - m_prevTicks; // difference between previous and current
        m_prevTicks = m_startFrameTicks; // store previous value
        m_totalDeltaTime = m_frameTime / DESIRED_FRAMETIME;
    }

    /**
     * @brief end perform fps calculation based on the difference from start to end
     * 
     * @return float approximated FPS value
     */
    float FPSLimiter::endFrame()
    {
        static const int NUM_SAMPLES = 10; // samples to average
        static float frameTimes[NUM_SAMPLES]; // array of samples
        static int currentFrame = 0; // place holder
        int count;
        float frameTimeAverage = 0;
        float currentTicks = SDL_GetTicks(); // get current ticks

        frameTimes[currentFrame % NUM_SAMPLES] = m_frameTime; // reset at 10
        currentFrame++;

        // if less than ten samples, average the current number, else average with 10
        if(currentFrame < NUM_SAMPLES) { count = currentFrame; }
        else { count = NUM_SAMPLES; }

        for(int i = 0; i < count; i++) { frameTimeAverage += frameTimes[i]; } // sum frame times
        frameTimeAverage /= count; // divide to get actual result

        if(frameTimeAverage > 0) { m_fps = 1000.0f / frameTimeAverage; } // set actual FPS
        else { m_fps = 0.0f; } // initialization display

        // get difference from beginning of frame and now
        float frameTicks = static_cast<float>(SDL_GetTicks() - m_startFrameTicks);

        // if estimate FPS > frameTicks, slow it down
        if(1000.0f / m_maxFPS > frameTicks)
        {
            SDL_Delay(static_cast<Uint32>(1000.0f / m_maxFPS - frameTicks)); // this is responsible for movement delay/jitter
        }

        return m_fps;
    }
}
