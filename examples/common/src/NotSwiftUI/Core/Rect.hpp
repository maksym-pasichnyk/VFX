//
// Created by Maksym Pasichnyk on 09.02.2023.
//

#pragma once

#include "Size.hpp"
#include "Point.hpp"

struct Rect {
    Point origin = Point::zero();
    Size size = Size::zero();
};