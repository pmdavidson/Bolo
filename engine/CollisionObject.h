#ifndef COLLISION_OBJECT_H
#define COLLISION_OBJECT_H
#include "GameObject.h"

namespace CMPUT350
{
    enum class ShapeType {
    kLine, kCircle, kRect
    };


    union ShapeUnion {
        ShapeUnion(Line l) : line(l) {}
        ShapeUnion(Circle c) : circle(c) {}
        ShapeUnion(Rect r) : rect(r) {}
        Line line;
        Circle circle;
        Rect rect; // Axis-aligned rectangle
    };


    struct Shape {
Shape(const Line &l) : shape(l), t(ShapeType::kLine) {
    std::cout << "[DEBUG] Shape(Line) created\n";
}
Shape(const Rect &r) : shape(r), t(ShapeType::kRect) {
    std::cout << "[DEBUG] Shape(Rect) created\n";
}
Shape(const Circle &c) : shape(c), t(ShapeType::kCircle) {
    std::cout << "[DEBUG] Shape(Circle) created\n";
}

        ShapeUnion shape;
        ShapeType t;
    };

    class CollisionObject : public GameObject{
    public:
        virtual ~CollisionObject() = default;

        // Called when a collision with another object occurs
        virtual void CollisionEnter(const std::shared_ptr<CollisionObject>& obj) = 0;
        virtual void CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
            {CollisionEnter(obj);}

        // Returns the object's current bounding box
        virtual const Rect& GetBounds() = 0;

        // Indicates that object does not move. Will only have collisions checked
        // against non-static objects.
        virtual bool IsStatic() const = 0;
        
        // Return the internal shapes in the object. Shapes can be:
        // lines, points/circles, or axis-aligned rectangles
        virtual const std::vector<Shape> &GetShapes() = 0;
        
        bool ShapeIntersect(const Shape &a, const Shape &b, Point2D *out);
        bool CollidesWith(const std::shared_ptr<CollisionObject> &other, Point2D *out = nullptr);
    };
}

#endif