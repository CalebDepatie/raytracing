#pragma once

#include <vector>
#include <memory>

class ray;
class point;
class object;

auto rayCast(ray r, const std::vector<std::shared_ptr<object>> scene, int bounces) -> point;
