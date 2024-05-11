// Header for SFML unit tests.
//
// For a new graphics module test case, include this header.
// This ensures that string conversions are visible and can be used by Catch2 for debug output.

#pragma once

#include <SystemUtil.hpp>
#include <iosfwd>

namespace sf
{
struct BlendMode;
struct StencilMode;
class Color;
class Transform;

template <typename>
class Rect;

std::ostream& operator<<(std::ostream& os, const BlendMode& blendMode);
std::ostream& operator<<(std::ostream& os, const StencilMode& stencilMode);
std::ostream& operator<<(std::ostream& os, const Color& color);
std::ostream& operator<<(std::ostream& os, const Transform& transform);

template <typename T>
std::ostream& operator<<(std::ostream& os, const Rect<T>& rect);
} // namespace sf

bool operator==(const sf::Transform& lhs, const Approx<sf::Transform>& rhs);

template <typename T>
bool operator==(const sf::Rect<T>& lhs, const Approx<sf::Rect<T>>& rhs)
{
    return lhs.left == Approx(rhs.value.left) && lhs.top == Approx(rhs.value.top) &&
           lhs.width == Approx(rhs.value.width) && lhs.height == Approx(rhs.value.height);
}
