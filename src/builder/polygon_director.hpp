#pragma once
#include <memory>
#include "polygon_builder.hpp"
#include "../model/polygon.hpp"

class PolygonDirector {
public:
    static std::unique_ptr<Polygon> createDemoPolygon1();
    static std::unique_ptr<Polygon> createDemoPolygon2();
};