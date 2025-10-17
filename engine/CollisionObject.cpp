#include "CollisionObject.h"
#include <algorithm>

namespace CMPUT350
{
    bool CollisionObject::ShapeIntersect(const Shape &a, const Shape &b, Point2D *out) {
        const auto typeA = a.t;
        const auto typeB = b.t;

        const auto &shapeA = a.shape;
        const auto &shapeB = b.shape;

        // LINE vs LINE
        if (typeA == ShapeType::kLine && typeB == ShapeType::kLine) {
            return shapeA.line.Crosses(shapeB.line, *out);
        }

        // LINE vs CIRCLE or CIRCLE vs LINE
        if (typeA == ShapeType::kLine && typeB == ShapeType::kCircle) {
            return LineIntersectsCircle(shapeA.line, shapeB.circle, out);
        }
        if (typeA == ShapeType::kCircle && typeB == ShapeType::kLine) {
            return LineIntersectsCircle(shapeB.line, shapeA.circle, out);
        }

        // LINE vs RECT or RECT vs LINE
        if (typeA == ShapeType::kLine && typeB == ShapeType::kRect) {
            return LineIntersectsRect(shapeA.line, shapeB.rect, out);
        }
        if (typeA == ShapeType::kRect && typeB == ShapeType::kLine) {
            return LineIntersectsRect(shapeB.line, shapeA.rect, out);
        }

        // CIRCLE vs CIRCLE
        if (typeA == ShapeType::kCircle && typeB == ShapeType::kCircle) {
            return CircleIntersectsCircle(shapeA.circle, shapeB.circle, out);
        }

    // CIRCLE vs RECT or RECT vs CIRCLE
    if (typeA == ShapeType::kCircle && typeB == ShapeType::kRect) {
        return CircleIntersectsRect(shapeA.circle, shapeB.rect, out);
    }
    if (typeA == ShapeType::kRect && typeB == ShapeType::kCircle) {
        return CircleIntersectsRect(shapeB.circle, shapeA.rect, out);
    }

    // RECT vs RECT
    if (typeA == ShapeType::kRect && typeB == ShapeType::kRect) {
        if (shapeA.rect.intersects(shapeB.rect)) {
            // Calculate collision point as center of intersection area
            if (out) {
                float leftEdge = std::max(shapeA.rect.topLeft.x, shapeB.rect.topLeft.x);
                float rightEdge = std::min(shapeA.rect.topLeft.x + shapeA.rect.width, 
                                          shapeB.rect.topLeft.x + shapeB.rect.width);
                float topEdge = std::max(shapeA.rect.topLeft.y, shapeB.rect.topLeft.y);
                float bottomEdge = std::min(shapeA.rect.topLeft.y + shapeA.rect.height, 
                                           shapeB.rect.topLeft.y + shapeB.rect.height);
                
                *out = Point2D((leftEdge + rightEdge) / 2.0f, (topEdge + bottomEdge) / 2.0f);
            }
            return true;
        }
        return false;
    }
    return false;
    }

    bool CollisionObject::CollidesWith(const std::shared_ptr<CollisionObject> &other, Point2D *out) {
        const auto &myShapes = this->GetShapes();
        const auto &theirShapes = other->GetShapes();

        for (const Shape &a : myShapes) {
            for (const Shape &b : theirShapes) {
                if (ShapeIntersect(a, b, out)) {
                    return true;
                }
            }
        }
        return false;
    }
}