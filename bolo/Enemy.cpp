#include <cassert>
#include "../engine/MathUtil.h"
#include "../engine/CollisionObject.h"
#include "../engine/NotificationManager.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Explosion.h"
#include "Wall.h"
#include "Base.h"
#include <assert.h>
#include <cstdlib>

namespace CMPUT350
{

    Enemy::Enemy(CMPUT350::Point2D loc, CMPUT350::Point2D direction, int level) : mPosition(loc),
                                                                       mPreviousPosition(loc),
                                                                       mDirection(direction),
                                                                       mSpawnPosition(loc),
                                                                       mLevel(level)
    {
        // Normalize initial direction
        mDirection.Normalize();
        
        if (mLevel == 1)
        {
            mFireChance = 250;
            mBulletSpeed = 30.0f;
            mArmColor = CMPUT350::RGBColor(255, 255, 0);    // Yellow
            mBarrelColor = CMPUT350::RGBColor(0, 255, 255); // Cyan
        }
        else if (mLevel == 3)
        {
            mFireChance = 150;
            mBulletSpeed = 35.0f;
            mArmColor = CMPUT350::RGBColor(255, 0, 0);      // Red
            mBarrelColor = CMPUT350::RGBColor(0, 255, 255); // Cyan
        }
        else // Level 5
        {
            mFireChance = 150;
            mBulletSpeed = 40.0f;
            mArmColor = CMPUT350::RGBColor(128, 0, 255);    // Purple
            mBarrelColor = CMPUT350::RGBColor(128, 0, 255); // Purple
        }
        
        UpdateBounds();
    }

    void Enemy::Update(GameContext *context)
    {
        GameObject::Update(context);

        if (!mIsAlive)
            return;

        // Decrement collision cooldown
        if (mCollisionCooldown > 0)
            mCollisionCooldown--;

        if (mSecondShotTimer > 0)
        {
            mSecondShotTimer--;
            if (mSecondShotTimer == 0)
            {
                // Fire the second bullet
                CMPUT350::Point2D bulletVelocity = mDirection * mBulletSpeed;
                auto bullet = std::make_shared<Bullet>(this, mPosition, bulletVelocity, 3.0f);
                if (context && context->EngineContext)
                {
                    mLastBullet = bullet;
                    context->EngineContext->AddGameObject(bullet);
                }
            }
        }

        // Check if enemy has exited base
        if (!mHasExitedBase)
        {
            float distanceFromSpawn = mPosition.Distance(mSpawnPosition);
            if (distanceFromSpawn > 102.0f) // Base radius
            {
                mHasExitedBase = true;
            }
        }

        // Random behavior only after exiting base
        if (mHasExitedBase)
        {
            // 1/400 chance to change direction
            if ((std::rand() % 400) == 0)
            {
                TurnRandomly();
            }
            
            // 1/250 chance to shoot bullet
            if ((std::rand() % mFireChance) == 0)
            {
                FireBullet(context);
            }
        }

        // Handle movement
        if (mCollisionCooldown == 0)
            UpdateMovement();

        // Update bounding box
        UpdateBounds();
    }

    void Enemy::LateUpdate(GameContext *context)
    {
        GameObject::LateUpdate(context);

        // Spawn explosion when enemy dies
        if (!mIsAlive && !mExplosionSpawned)
        {
            mExplosionSpawned = true;
            if (mLastContext && mLastContext->EngineContext)
            {
                auto explosion = std::make_shared<Explosion>(mPosition, 15);
                mLastContext->EngineContext->AddGameObject(explosion);

                if (mLastContext->NotificationContext)
                {
                    mLastContext->NotificationContext->Notify("ENEMY_DESTROYED");
                }
            }
            mLastContext = nullptr;
        }
    }

    void Enemy::RenderBackground(GameContext *context)
    {
        if (!mIsAlive)
            return;

        // Draw Y-shape pointing in direction of travel
        float armLength = mBarrelLength;
        float armAngle = CMPUT350::DegToRad(45.0f);

        // Get perpendicular to direction for the arms
        CMPUT350::Point2D perpLeft = CMPUT350::Rotate(mDirection, armAngle);
        CMPUT350::Point2D perpRight = CMPUT350::Rotate(mDirection, -armAngle);

        // Left arm
        CMPUT350::Point2D leftArmEnd = mPosition - perpLeft * armLength;
        context->ScreenContext->DrawLine(mPosition, leftArmEnd, 4.0f, mArmColor);

        // Right arm
        CMPUT350::Point2D rightArmEnd = mPosition - perpRight * armLength;
        context->ScreenContext->DrawLine(mPosition, rightArmEnd, 4.0f, mArmColor);

        // Draw barrel pointing forward
        CMPUT350::Point2D barrelEnd = mPosition + mDirection * mBarrelLength;
        context->ScreenContext->DrawLine(mPosition, barrelEnd, 4.0f, mBarrelColor);

        // Level 5: Add a line across the enemy
        if (mLevel == 5)
        {
            // Draw a perpendicular line across the center
            CMPUT350::Point2D perpDir = CMPUT350::Point2D(-mDirection.y, mDirection.x);
            CMPUT350::Point2D lineStart = mPosition - perpDir * (mBarrelLength * 0.8f);
            CMPUT350::Point2D lineEnd = mPosition + perpDir * (mBarrelLength * 0.8f);
            context->ScreenContext->DrawLine(lineStart, lineEnd, 4.0f, CMPUT350::RGBColor(0, 255, 0)); // Green line
        }
    }

    void Enemy::RenderForeground(GameContext *context)
    {
    }

    bool Enemy::HandleKeyEvent(CMPUT350::GameContext *context, char key)
    {
        // Enemies don't handle keyboard input
        return false;
    }

    void Enemy::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
    {
        // Approximate point from midpoint
        CollisionEnter(obj, mPosition);
    }

    void Enemy::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
    {
        if (!mIsAlive)
            return;

        // Ignore own bullet
        if (mLastBullet.lock().get() == obj.get())
        {
            return;
        }

        // Check if collision is with wall or another enemy
        auto wall = std::dynamic_pointer_cast<Wall>(obj);
        auto enemy = std::dynamic_pointer_cast<Enemy>(obj);
        auto base = std::dynamic_pointer_cast<Base>(obj);

        // Handle base collisions
        if (base)
        {
            // If we haven't exited the base yet
            if (!mHasExitedBase)
            {
                return;
            }
            // If we have exited the base
            else
            {
                // Undo last move
                mPosition = mPreviousPosition;
                UpdateBounds();

                // Turn when hitting base after exiting
                TurnRandomly();

                // Set collision cooldown to pause movement for a couple frames
                mCollisionCooldown = 2;
                return;
            }
        }

        if (wall)
        {
            // Undo last move
            mPosition = mPreviousPosition;
            UpdateBounds(); // Update bounds after position change

            // Only turn when hitting wall if we've exited the base
            if (mHasExitedBase)
            {
                TurnRandomly();
            }

            // Set collision cooldown to pause movement for a couple frames
            mCollisionCooldown = 2;
        }
        else if (enemy)
        {
            // For enemy-enemy collisions, try to avoid getting stuck
            // Move back slightly and turn away from the other enemy
            mPosition = mPreviousPosition;
            UpdateBounds();

            // Only turn when hitting other enemies if we've exited the base
            if (mHasExitedBase)
            {
                // Turn away from the other enemy instead of random turn
                Point2D awayFromEnemy = mPosition - mPreviousPosition;
                float distance = std::sqrt(awayFromEnemy.x * awayFromEnemy.x + awayFromEnemy.y * awayFromEnemy.y);
                if (distance > 0)
                {
                    awayFromEnemy.x /= distance;
                    awayFromEnemy.y /= distance;
                    // Add some randomness to avoid perfect alignment
                    float randomAngle = (std::rand() % 60 - 30) * 3.14159f / 180.0f; // ±30 degrees
                    awayFromEnemy = Rotate(awayFromEnemy, randomAngle);
                    mDirection = awayFromEnemy;
                }
                else
                {
                    // Fallback to random turn if we can't determine direction
                    TurnRandomly();
                }
            }

            // Set collision cooldown to pause movement for a couple frames
            mCollisionCooldown = 2;
        }
        else
        {
            // Collision with anything else (bullets, player, explosions) - explode
            mIsAlive = false;
        }
    }

    const Rect &Enemy::GetBounds()
    {
        return mBounds;
    }

    const std::vector<Shape> &Enemy::GetShapes()
    {
        mShapes.clear();
        mShapes.push_back(Circle(mPosition, mRadius));
        return mShapes;
    }

    bool Enemy::IsAlive()
    {
        return mIsAlive;
    }

    void Enemy::UpdateMovement()
    {
        // Save previous position for undo
        mPreviousPosition = mPosition;

        // Move in current direction at 1 pixel per frame
        mPosition.x += mDirection.x * mSpeed;
        mPosition.y += mDirection.y * mSpeed;
    }

    void Enemy::FireBullet(GameContext *context)
    {
        if (!mIsAlive || !context || !context->EngineContext)
            return;

        CMPUT350::Point2D bulletVelocity = mDirection * mBulletSpeed;

        auto bullet = std::make_shared<Bullet>(this, mPosition, bulletVelocity, 3.0f);

        // Track the most recent bullet
        mLastBullet = bullet;

        context->EngineContext->AddGameObject(bullet);

        // Level 5 enemies fire a second bullet after a short delay
        if (mLevel == 5)
        {
            mSecondShotTimer = 20; // Delay of 20 frames 
        }
    }

    void Enemy::UpdateBounds()
    {
        // Circular bounding box for collision detection
        mBounds = CMPUT350::Rect(mPosition, mRadius);
    }

    void Enemy::TurnRandomly()
    {
        // Turn by 45, 90, or 135 degrees randomly to avoid getting stuck
        int turnSteps = 1 + (std::rand() % 3); // 1, 2, or 3 steps of 45 degrees
        float turnAngle = CMPUT350::DegToRad(45.0f * turnSteps);

        // Turn left or right randomly
        if (std::rand() % 2 == 0)
        {
            turnAngle = -turnAngle;
        }

        mDirection = CMPUT350::Rotate(mDirection, turnAngle);
        mDirection.Normalize();
    }
}