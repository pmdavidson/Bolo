#include "Bullet.h"
#include "Explosion.h"
#include "../engine/CollisionObject.h"
#include "Enemy.h"
#include "Base.h"
#include "GameContext.h"
#include "DrawContext.h"
#include <algorithm> // for std::min, std::max in Update()

namespace CMPUT350
{
    // static cast collision object DANGEROUS, make sure always collision object for GetBounds collisions
    Bullet::Bullet(GameObject *parent, Point2D location, Point2D heading, float radius)
        : mParent(static_cast<CollisionObject *>(parent)), mPrevPosition(location), mPosition(location),
          mHeading(heading), mRadius(radius)
    {
        mBounds = Rect(location, radius); // Initial bounding box
    }

    void Bullet::Update(GameContext *context)
    {
        GameObject::Update(context);
        mPrevPosition = mPosition;
        mPosition += mHeading;

        mLifetime -= context->DeltaTime;
        if (mLifetime <= 0.0f)
        {
            mIsAlive = false;
            return;
        }

        // Recompute AABB from previous and current positions
        Point2D topLeft(
            std::min(mPrevPosition.x, mPosition.x) - mRadius,
            std::min(mPrevPosition.y, mPosition.y) - mRadius);
        float width = std::abs(mPrevPosition.x - mPosition.x) + 2 * mRadius;
        float height = std::abs(mPrevPosition.y - mPosition.y) + 2 * mRadius;
        mBounds = Rect(topLeft, width, height);
    }

    void Bullet::RenderBackground(GameContext *context)
    {
        context->ScreenContext->DrawCircle(mPosition, mRadius, Colors::yellow);
    }

    void Bullet::RenderForeground(GameContext *context)
    {
    }

    void Bullet::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
    {
        if (!obj)
            return;
        CollisionEnter(obj, mPosition);
    }

    void Bullet::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
    {
        if (!mIsAlive)
            return;

        if (!obj)
            return;

        // Ignore collision with parent tank
        if (mParent && obj.get() == mParent)
            return;

        mIsAlive = false;

        // Don't spawn explosion for enemies
        auto enemy = std::dynamic_pointer_cast<Enemy>(obj);
        
        if (!enemy && mLastContext && mLastContext->EngineContext)
        {
            auto newExplosion = std::make_shared<Explosion>(collisionPoint, 10);
            mLastContext->EngineContext->AddGameObject(newExplosion);
        }
        
        mLastContext = nullptr;
        mParent = nullptr;
    }

    const Rect &Bullet::GetBounds()
    {
        return mBounds;
    }

    const std::vector<Shape> &Bullet::GetShapes()
    {
        mShapes.clear();
        
        // Rectangle along bullet's path to prevent tunneling through walls
        // This is the swept bounding box from previous to current position
        mShapes.emplace_back(Rect(mBounds));
        
        // Circle at current position for precise collision detection
        mShapes.emplace_back(Circle(mPosition, mRadius));
        
        return mShapes;
    }
}
