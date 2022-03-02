#ifndef RANDOMENGINE_H
#define RANDOMENGINE_H

#include "common/types.h"
#include <random>

/*
    Ref:
        https://www.guyrutenberg.com/2014/05/03/c-mt19937-example/
        https://stackoverflow.com/questions/19036141/vary-range-of-uniform-int-distribution
        http://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine
        http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
        http://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
*/

namespace CGameEngine
{
    class RandomEngine
    {
        public:
            RandomEngine() {}
            virtual ~RandomEngine() {}
            int roll100() { return std::uniform_int_distribution<int>{1,100}(m_rand); }
            int rollInt(int& maxVal) { return std::uniform_int_distribution<int>{1,maxVal}(m_rand); }
            int rollInt(int& minVal, int& maxVal) { return std::uniform_int_distribution<int>{minVal,maxVal}(m_rand); }
            float rollReal() { return std::uniform_real_distribution<float>{0.0f,1.0f}(m_rand); }
            float rollReal(float& maxVal) { return std::uniform_real_distribution<float>{1.0f,maxVal}(m_rand); }
            float rollReal(float& minVal, float& maxVal) { return std::uniform_real_distribution<float>{minVal,maxVal}(m_rand); }

        private:
            std::mt19937 m_rand{std::random_device{}()};
    };
}

#endif // RANDOMENGINE_H
