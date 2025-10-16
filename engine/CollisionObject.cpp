#include "CollisionObject.h"

namespace CMPUT350
{
    void CollisionObject::CollisionEnter(const std::shared_ptr<CollisionObject>& obj, const Point2D& collisionPoint) {
        // Default: just call the basic one
        CollisionEnter(obj);
    }
}
