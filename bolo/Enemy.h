#ifndef ENEMY_H
#define ENEMY_H
#include "CollisionObject.h"
#include "MathUtil.h"
#include "GameContext.h"
namespace CMPUT350
{
    class Bullet;
    class Enemy : public CollisionObject
    {
    public:
        Enemy(Point2D loc, Point2D direction = Point2D(0, -1));

        void Update(GameContext *context) override;
        void LateUpdate(GameContext *context) override;
        void RenderBackground(GameContext *context) override;
        void RenderForeground(GameContext *context) override;
        bool HandleKeyEvent(GameContext *context, char key) override;
        const Rect &GetBounds() override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint) override;
        bool IsStatic() const override { return false; }
        const std::vector<Shape> &GetShapes() override;

        bool IsAlive() override;

    private:
        Point2D mPosition;
        Point2D mPreviousPosition; // For undo on collision
        Point2D mDirection; // Normalized direction of travel
        float mSpeed = 1.0f; // 1 pixel per frame
        float mRadius = 20.0f; // Enemy radius
        float mBarrelLength = 15.0f; // Y-shape barrel length
        Rect mBounds;
        bool mHasExitedBase = false; // Track if enemy has left spawn base
        Point2D mSpawnPosition; // Where enemy was spawned

        void UpdateMovement();
        void FireBullet(GameContext *context);
        void UpdateBounds();
        void TurnRandomly(); // Turn left or right randomly

        bool mIsAlive = true;
        bool mExplosionSpawned = false;
        std::weak_ptr<Bullet> mLastBullet; // Track most recently fired bullet
        int mCollisionCooldown = 0; // Prevent immediate re-collision
        std::vector<Shape> mShapes;
    };
}
#endif
