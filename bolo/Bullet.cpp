#include "Bullet.h"
#include "Explosion.h"
#include "../engine/CollisionObject.h"
#include "Enemy.h"
#include "Base.h"
#include "GameContext.h"
#include "DrawContext.h"

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

    // void Bullet::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
    // {
    //     // Approximate point from midpoint
    //     Point2D guess = (mPrevPosition + mPosition) * 0.5f;
    //     CollisionEnter(obj, guess);
    // }
    void Bullet::CollisionEnter(const std::shared_ptr<CollisionObject> &obj) {
    std::cout << "[Bullet] CollisionEnter with object at " << obj.get() << "\n";
    std::cout << "  Shape count: " << obj->GetShapes().size() << "\n";
    std::cout << "  First shape type: " << static_cast<int>(obj->GetShapes()[0].t) << "\n";
    CollisionEnter(obj, mPosition);

}

    void Bullet::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
    {
        if (!mIsAlive)
            return;

        // Ignore collision with parent tank if bullet still inside its bounds
        if (obj.get() == mParent) //&& mBounds.intersects(mParent->GetBounds())
            return;

    // // Check if collision is with enemy or base
    // auto enemy = std::dynamic_pointer_cast<Enemy>(obj);
    // auto base = std::dynamic_pointer_cast<Base>(obj);

        // Kill bullet
        mIsAlive = false;

        // Spawn explosion on bullet collision
        if (mLastContext && mLastContext->EngineContext)
        {
            auto explosion = std::make_shared<Explosion>(mPosition, 10);
            mLastContext->EngineContext->AddGameObject(explosion);
        }
        // Clear context reference to prevent dangling pointer
        mLastContext = nullptr;

        // Always clear parent to allow future bullets to hit this tank again
        mParent = nullptr;
        }

    const Rect &Bullet::GetBounds()
    {
        return mBounds;
    }

    const std::vector<Shape> &Bullet::GetShapes()
    {
        mShapes.clear();
        mShapes.emplace_back(Circle(mPosition, mRadius));
        return mShapes;
    }
}
