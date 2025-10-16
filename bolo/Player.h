#ifndef PLAYER_H
#define PLAYER_H
#include "../engine/CollisionObject.h"
#include "MathUtil.h"
#include "GameContext.h"
namespace CMPUT350
{
	class Bullet;

	class Player : public CollisionObject
	{
	public:
		Player(Point2D loc);

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
		Point2D mDirection;
		Point2D mTurretDirection;
		float mSpeed;

		float mTankSize;
		float mTurretLength;
		Rect mBounds;
		std::vector<Shape> mShapes;

		void UpdateMovement();
		void FireBullet(GameContext *context);
		void UpdateBounds();

		bool mIsAlive = true;
		std::weak_ptr<Bullet> mLastBullet; // Track most recently fired bullet
		int mFireCooldown = 0;			   // Cooldown counter for firing bullets
	};
}
#endif
