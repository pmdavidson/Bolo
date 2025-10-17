#include "Wall.h"
#include "DrawContext.h"
#include "GameContext.h"
#include "Explosion.h"

namespace CMPUT350
{

    Wall::Wall(Point2D origin, float length, tWallDirection dir, float width)
        : mOrigin(origin), mLength(length), mDir(dir), mWidth(width)
    {
        //set wall bounds based on direction
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
        //walls render as thick gray lines
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
        //intentionally left blank, wall does not draw in foreground
    }

    void Wall::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
    {
        //no-op: override for interface
    }

    void Wall::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
    {
        //no-op: override for interface
    }

    const Rect& Wall::GetBounds()
    {
        return mBounds;
    }

    const std::vector<Shape> &Wall::GetShapes()
    {
        mShapes.clear();

        if (mDir == kHorizontal)
        {
            //bounding box for broad phase
            mShapes.emplace_back(Rect(mOrigin, mLength, mWidth));
            
            //line shape for narrow phase
            Point2D p1 = mOrigin;
            Point2D p2(mOrigin.x + mLength, mOrigin.y);
            mShapes.emplace_back(Line(p1, p2));
        }
        else
        {
            mShapes.emplace_back(Rect(mOrigin, mWidth, mLength));

            Point2D p1 = mOrigin;
            Point2D p2(mOrigin.x, mOrigin.y + mLength);
            mShapes.emplace_back(Line(p1, p2));
        }

        return mShapes;
    }
}
