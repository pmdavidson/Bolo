#ifndef EXPLOSION_H
#define EXPLOSION_H

#pragma once

#include "CollisionObject.h"
#include "MathUtil.h"

namespace CMPUT350
{
    class Explosion : public CollisionObject
    {
    public:
        Explosion(Point2D location, int radius);

        void LateUpdate(GameContext *context) override;
        void RenderBackground(GameContext *context) override;
        void RenderForeground(GameContext *context) override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint) override;
        
        const Rect &GetBounds() override;
        bool IsStatic() const override {return false;}
        const std::vector<Shape> &GetShapes() override;

    private:
        Point2D mLocation;
        int mRadius;
        Rect mBounds;

        std::vector<Shape> mShapes;
    };
}

#endif