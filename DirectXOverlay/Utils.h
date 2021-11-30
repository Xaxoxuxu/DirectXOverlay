#pragma once
#include <random>
#include <thread>

class Utils
{
public:
    static int GetRandomInteger(const int& min, const int& max)
    {
        const std::uniform_int_distribution<> distribution(min, max);
        std::mt19937 m_mersenneTwister{ static_cast<std::uint32_t>(
              std::chrono::high_resolution_clock::now().time_since_epoch().count()
          ) };
        return distribution(m_mersenneTwister);
    }
    static double GetRandomDouble(const double& min, const double& max)
    {
        const std::uniform_real_distribution<> distribution(min, max);
        std::mt19937 m_mersenneTwister{ static_cast<std::uint32_t>(
              std::chrono::high_resolution_clock::now().time_since_epoch().count()
          ) };
        return distribution(m_mersenneTwister);
    }
};