#include "Wall.h"
#include "DrawContext.h"
#include "GameContext.h"
#include "Explosion.h"

namespace CMPUT350
{

    Wall::Wall(Point2D origin, float length, tWallDirection dir, float width)
        : mOrigin(origin), mLength(length), mDir(dir), mWidth(width)
    {
        // Set wall bounds vertically or horizontally
        if (dir == kHorizontal)
        {
            mBounds = Rect(origin, static_cast<int>(length), static_cast<int>(width));

        }
        else
        {
            mBounds = Rect(origin, static_cast<int>(width), static_cast<int>(length));

        }
    }

    void Wall::RenderBackground(GameContext *context)
    {
        // Walls render in background
        if (mDir == kHorizontal)
        {
            context->ScreenContext->DrawLine(
                mOrigin, Point2D(mOrigin.x + mLength, mOrigin.y), mWidth, Colors::gray);
        }
        else
        {
            context->ScreenContext->DrawLine(
                mOrigin, Point2D(mOrigin.x, mOrigin.y + mLength), mWidth, Colors::gray);
        }
    }

    void Wall::RenderForeground(GameContext *context)
    {
    }

    void Wall::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
    {
    }
    void Wall::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint) {}

    const Rect& Wall::GetBounds() {
        return mBounds;
    }


const std::vector<Shape> &Wall::GetShapes()
{
    mShapes.clear();

    // Return wall as a rectangle instead of a line for simpler collision detection
    if (mDir == kHorizontal)
    {
        // Horizontal wall: full length, width in height
        Rect wallRect(mOrigin, mLength, mWidth);
        mShapes.emplace_back(Shape(wallRect));
    }
    else
    {
        // Vertical wall: width in width, full length in height
        Rect wallRect(mOrigin, mWidth, mLength);
        mShapes.emplace_back(Shape(wallRect));
    }

    return mShapes;
}



}
