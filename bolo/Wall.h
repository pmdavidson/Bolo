#ifndef WALL_H
#define WALL_H

#pragma once

#include "../engine/CollisionObject.h"
#include "MathUtil.h"

namespace CMPUT350
{
    enum tWallDirection
    {
        kHorizontal,
        kVertical
    };

    class Wall : public CollisionObject
    {
    public:
        Wall(Point2D origin, float length, tWallDirection dir, float width);

        void RenderBackground(GameContext *context) override;
        void RenderForeground(GameContext *context) override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override;
        void CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint) override;
        bool IsAlive() override { return true; }

        const Rect &GetBounds() override;
        bool IsStatic() const override { return true; }
        const std::vector<Shape> &GetShapes() override;

    private:
        Point2D mOrigin;
        float mLength;
        float mWidth;
        tWallDirection mDir;

        Rect mBounds;
        std::vector<Shape> mShapes;
    };
}

#endif
