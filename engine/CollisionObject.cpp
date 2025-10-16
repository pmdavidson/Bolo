#include "CollisionObject.h"
#include <typeinfo>
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
            if (!CircleIntersectsRect(shapeA.circle, shapeB.rect, out)) {
                // std::cout << "[DEBUG] Bullet at " << shapeA.circle.center << " (r=" << shapeA.circle.radius
                //         << ") missed wall at " << shapeB.rect.topLeft << " w=" << shapeB.rect.width << " h=" << shapeB.rect.height << "\n";
            }
            return CircleIntersectsRect(shapeA.circle, shapeB.rect, out);
        }
        if (typeA == ShapeType::kRect && typeB == ShapeType::kCircle) {
            if (!CircleIntersectsRect(shapeB.circle, shapeA.rect, out)) {
                // std::cout << "[DEBUG] Bullet at " << shapeB.circle.center << " (r=" << shapeB.circle.radius
                //         << ") missed wall at " << shapeA.rect.topLeft << " w=" << shapeA.rect.width << " h=" << shapeA.rect.height << "\n";
            }
            return CircleIntersectsRect(shapeB.circle, shapeA.rect, out);
        }

        // RECT vs RECT
        if (typeA == ShapeType::kRect && typeB == ShapeType::kRect) {
            return shapeA.rect.intersects(shapeB.rect);
        }
        return false;
    }

        // bool CollisionObject::CollidesWith(const std::shared_ptr<CollisionObject> &other, Point2D *out) {
    bool CollisionObject::CollidesWith(const std::shared_ptr<CollisionObject> &other, Point2D *out) {
        const auto &myShapes = this->GetShapes();
        const auto &theirShapes = other->GetShapes();

        for (const Shape &a : myShapes) {
            for (const Shape &b : theirShapes) {
                // std::cout << "[COLLISION CHECK] My shape type = " << (int)a.t
                //         << ", Their shape type = " << (int)b.t << std::endl;
                if (ShapeIntersect(a, b, out)) {
        //             std::cout << "[COLLISION DETECTED]" << std::endl;
        //             std::cout << "[COLLISION] objA: " << typeid(*this).name()
        //   << ", objB: " << typeid(*other).name() << std::endl;

                    return true;
                }
            }
        }
        return false;
    }
}