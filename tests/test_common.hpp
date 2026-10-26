// Tiny helpers shared by the unit tests (no test framework needed).
#pragma once
#include <cmath>
#include <cstdio>
#include <SFML/System/Vector2.hpp>
#include "utils/vec_math.hpp"

static int failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);      \
            ++failures;                                                      \
        }                                                                    \
    } while (0)

static bool near(sf::Vector2f a, sf::Vector2f b, float eps = 1e-3f) { return vec::length(a - b) <= eps; }
static bool near(float a, float b, float eps = 1e-3f) { return std::abs(a - b) <= eps; }

using V = sf::Vector2f;
