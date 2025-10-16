#include "Explosion.h"
#include "GameContext.h"
#include "DrawContext.h"

namespace CMPUT350
{
    Explosion::Explosion(Point2D location, int radius)
        : mLocation(location), mRadius(radius)
    {
        mBounds = Rect(location, static_cast<float>(radius));
    }

    // Explosions are in LateUpdate not Update
    void Explosion::LateUpdate(GameContext *context)
    {
        GameObject::LateUpdate(context);

        if (mRadius > 0)
        {
            mRadius--;
            mBounds = Rect(mLocation, static_cast<float>(mRadius));
            mShapes[0] = Circle(mLocation, static_cast<float>(mRadius));
        }
        // If explosion is gone then it's dead
        if (mRadius <= 0)
        {
            mIsAlive = false;
        }
    }

    void Explosion::RenderBackground(GameContext *context)
    {
        context->ScreenContext->DrawCircle(mLocation, static_cast<float>(mRadius), Colors::magenta);
    }

    void Explosion::RenderForeground(GameContext *context)
    {
    }

    void Explosion::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
    {
        // Explosions don't react to collisions
    }

    void Explosion::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
    {
        // Explosions don't react to collisions
    }

    const Rect &Explosion::GetBounds()
    {
        return mBounds;
    }

    const std::vector<Shape> &Explosion::GetShapes()
    {
        mShapes.clear();
        mShapes.emplace_back(Circle(mLocation, mRadius));
        return mShapes;
    }
}
