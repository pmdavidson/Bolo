#ifndef BASE_H
#define BASE_H
#include "CollisionObject.h"
#include "MathUtil.h"
#include "GameContext.h"
namespace CMPUT350
{
    class Bullet;

    class Base : public CollisionObject
    {
    public:
        Base(Point2D loc, int gameLevel = 1);

        void Update(GameContext *context) override;
        void LateUpdate(GameContext *context) override;
        void RenderBackground(GameContext *context) override;
        void RenderForeground(GameContext *context) override;
        const Rect &GetBounds() override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint) override;
        bool IsStatic() const override { return false; }
        const std::vector<Shape> &GetShapes() override;
        bool IsAlive() override;

    private:
        void UpdateBounds();
        int mHealth = 4;
        int mEnemySpawnCooldown = 0;
        int mBaseRegenCooldown = 0;
        Point2D mPosition;
        Rect mBounds;
        const float mBaseRadius = 102.0f;
        std::vector<Shape> mShapes;
        bool mExplosionSpawned = false;
        int mGameLevel; // Game level determines enemy types (1, 3, or 5)
    };
}
#endif
