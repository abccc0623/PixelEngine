#pragma once
#include "Type/PVector3.h"
class Rect
{
public:
    Rect() = default;
    ~Rect() = default;

    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    PVector3 GetCenter() const
    {
        return  PVector3(x + width * 0.5f, y + height * 0.5f, 0.0f);
    }
    
    bool Contains(const PVector3& point) const 
    {
        return (point.X >= Left() && point.X <= Right() &&
            point.Y >= Top() && point.Y <= Bottom());
    }

    bool Intersects(const Rect& other) const 
    {
        return (Left() < other.Right() && Right() > other.Left() &&
            Top() < other.Bottom() && Bottom() > other.Top());
    }

    float Left()   const { return x; }
    float Right()  const { return x + width; }
    float Top()    const { return y; }
    float Bottom() const { return y + height; }
};

