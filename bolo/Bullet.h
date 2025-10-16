#ifndef BULLET_H
#define BULLET_H

#pragma once

#include "CollisionObject.h"
#include "MathUtil.h"

namespace CMPUT350
{
    class Bullet : public CollisionObject
    {
    public:
        Bullet(GameObject *parent, Point2D location, Point2D heading, float radius = 3.0f);

        void Update(GameContext *context) override;
        void RenderBackground(GameContext *context) override;
        void RenderForeground(GameContext *context) override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint) override;
        const Rect &GetBounds() override;
        bool IsStatic() const override { return false; }
        const std::vector<Shape> &GetShapes() override;

    private:
        Point2D mPrevPosition;
        Point2D mPosition;
        Point2D mHeading;
        float mRadius;
        CollisionObject *mParent; // changed from GameObject type
        Rect mBounds;

        std::vector<Shape> mShapes;
        float mLifetime = 6.0f;
    };
}

#endif // BULLET_H