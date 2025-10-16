#include <cassert>
#include "../engine/MathUtil.h"
#include "../engine/CollisionObject.h"
#include "../engine/NotificationManager.h"
#include "Base.h"
#include "Bullet.h"
#include "Explosion.h"
#include "Enemy.h"
#include "Player.h"
#include "Wall.h"
#include "Bolo.h"
#include <assert.h>
#include <cstdlib>
#include <ctime>

namespace CMPUT350
{

    Base::Base(CMPUT350::Point2D loc)
        : mPosition(loc), mHealth(4), mEnemySpawnCooldown(60), mBaseRegenCooldown(60)
    {
        UpdateBounds();
        // Seed random number generator
        static bool seeded = false;
        if (!seeded)
        {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            seeded = true;
        }
    }

    void Base::Update(GameContext *context)
    {
        GameObject::Update(context);

        if (!mIsAlive)
            return;

        // Health regeneration every 2 seconds (60 frames at 30 fps)
        if (mBaseRegenCooldown > 0)
        {
            mBaseRegenCooldown--;
        }

        if (mBaseRegenCooldown <= 0)
        {
            if (mHealth < 4)
            {
                mHealth++;
                UpdateBounds(); // Update bounding box when health regenerates
            }
            mBaseRegenCooldown = 60; // Reset to 2 seconds
        }

        // Ensure health stays within valid bounds
        if (mHealth < 0)
            mHealth = 0;
        if (mHealth > 4)
            mHealth = 4;

        // Enemy spawning every 2 seconds
        if (mEnemySpawnCooldown > 0)
        {
            mEnemySpawnCooldown--;
        }
        if (mEnemySpawnCooldown <= 0 && context->EngineContext)
        {
            // Spawn enemy in random direction 4 directions
            int direction = std::rand() % 4;
            float spawnDistance = 20.f; // Spawn in base
            Point2D spawnPos;
            Point2D enemyDirection;

            switch (direction)
            {
            case 0: // North
                spawnPos = Point2D(mPosition.x, mPosition.y - spawnDistance);
                enemyDirection = Point2D(0, -1);
                break;
            case 1: // East
                spawnPos = Point2D(mPosition.x + spawnDistance, mPosition.y);
                enemyDirection = Point2D(1, 0);
                break;
            case 2: // South
                spawnPos = Point2D(mPosition.x, mPosition.y + spawnDistance);
                enemyDirection = Point2D(0, 1);
                break;
            case 3: // West
                spawnPos = Point2D(mPosition.x - spawnDistance, mPosition.y);
                enemyDirection = Point2D(-1, 0);
                break;
            }

            // Check if a wall is blocking the direction of travel
            // Check a point at the cell boundary in the spawn direction
            // Since bases are at cell centers (multiples of 256 + 128), check the cell edge
            Point2D checkPoint = mPosition + Point2D(enemyDirection.x * 128, enemyDirection.y * 128);

            bool wallBlocking = false;
            for (auto it = context->EngineContext->begin(); it != context->EngineContext->end(); ++it)
            {
                auto wall = std::dynamic_pointer_cast<Wall>(*it);
                if (wall)
                {
                    // Check if the check point is inside any wall's bounding box
                    if (wall->GetBounds().IsInside(checkPoint))
                    {
                        wallBlocking = true;
                        break;
                    }
                }
            }

            // Only spawn enemy if no wall is blocking
            if (!wallBlocking)
            {
                auto enemy = std::make_shared<Enemy>(spawnPos, enemyDirection);
                context->EngineContext->AddGameObject(enemy);
            }

            mEnemySpawnCooldown = 60; // Reset to 2 seconds
        }

        // Update bounding box
        UpdateBounds();
    }

    void Base::LateUpdate(GameContext *context)
    {
        GameObject::LateUpdate(context);

        // Spawn multiple explosions when Base dies
        if (!mIsAlive && !mExplosionSpawned)
        {
            mExplosionSpawned = true;
            // Checks if there is an existing context that corresponds to the engine
            if (mLastContext && mLastContext->EngineContext)
            {
                for (int i = 0; i < 4; i++)
                {
                    float offsetX = (std::rand() % 200 - 100) / 100.0f * mBaseRadius * 0.8f;
                    float offsetY = (std::rand() % 200 - 100) / 100.0f * mBaseRadius * 0.8f;
                    Point2D explosionPos(mPosition.x + offsetX, mPosition.y + offsetY);

                    int explosionRadius = 10 + (std::rand() % 21);

                    auto explosion = std::make_shared<Explosion>(explosionPos, explosionRadius);
                    mLastContext->EngineContext->AddGameObject(explosion);
                }

                if (mLastContext->NotificationContext)
                {
                    mLastContext->NotificationContext->Notify("BASE_DESTROYED");
                }
            }
            // Clear context reference to prevent dangling pointer
            mLastContext = nullptr;
        }
    }

    void Base::RenderBackground(GameContext *context)
    {
        if (!mIsAlive)
            return;

        // Red circle
        float heartRadius = mBaseRadius * 0.25f;                                                   // Heart is about 1/4 of base radius
        context->ScreenContext->DrawCircle(mPosition, heartRadius, CMPUT350::RGBColor(255, 0, 0)); // Red heart

        // White square outer wall (only if health > 0)
        if (mHealth > 0)
        {
            // Simple constant shrink per health point lost
            const float shrinkPerHealth = 30.0f;      // Each health loss shrinks square by 40px
            const float maxSize = mBaseRadius * 2.0f; // 204px at full health
            const float wallThickness = 30.0f;        // Constant wall thickness

            // Starts at maxSize, shrinks by constant amount per health lost
            float outerSize = maxSize - (shrinkPerHealth * (4 - mHealth));

            // Ensure outer size doesn't get too small
            float heartDiameter = mBaseRadius * 0.6f;
            if (outerSize < heartDiameter + 2.0f * wallThickness)
            {
                outerSize = heartDiameter + 2.0f * wallThickness;
            }

            float innerSize = outerSize - 2.0f * wallThickness;

            // Draw the white square as 4 rectangles (top, right, bottom, left)
            float halfOuter = outerSize / 2.0f;
            float halfInner = innerSize / 2.0f;

            // Top wall
            CMPUT350::Rect topWall(mPosition.x - halfOuter, mPosition.y - halfOuter, outerSize, wallThickness);
            context->ScreenContext->DrawRect(topWall, CMPUT350::RGBColor(255, 255, 255));

            // Bottom wall
            CMPUT350::Rect bottomWall(mPosition.x - halfOuter, mPosition.y + halfInner, outerSize, wallThickness);
            context->ScreenContext->DrawRect(bottomWall, CMPUT350::RGBColor(255, 255, 255));

            // Left wall
            CMPUT350::Rect leftWall(mPosition.x - halfOuter, mPosition.y - halfOuter, wallThickness, outerSize);
            context->ScreenContext->DrawRect(leftWall, CMPUT350::RGBColor(255, 255, 255));

            // Right wall
            CMPUT350::Rect rightWall(mPosition.x + halfInner, mPosition.y - halfOuter, wallThickness, outerSize);
            context->ScreenContext->DrawRect(rightWall, CMPUT350::RGBColor(255, 255, 255));
        }
    }

    void Base::RenderForeground(GameContext *context)
    {
    }

    void Base::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
    {
        // Use base position as collision point
        CollisionEnter(obj, mPosition);
    }

    void Base::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
    {
        if (!mIsAlive)
            return;

        // Ignore collisions with enemies
        auto enemy = std::dynamic_pointer_cast<Enemy>(obj);
        if (enemy)
            return;

        // Check if collision is with bullet or player
        auto bullet = std::dynamic_pointer_cast<Bullet>(obj);
        auto player = std::dynamic_pointer_cast<Player>(obj);

        // Bullets and players damage the base
        if (bullet || player)
        {
            // If health > 0, damage the outer wall
            if (mHealth > 0)
            {
                mHealth--;
                // Clamp health to prevent going below 0
                if (mHealth < 0)
                    mHealth = 0;
                UpdateBounds(); // Update bounding box when taking damage
            }
            else if (mHealth <= 0)
            {
                // Health is 0, outer wall is gone, destroy the base
                mIsAlive = false;
            }
        }
    }

    const Rect &Base::GetBounds()
    {
        return mBounds;
    }

    const std::vector<Shape> &Base::GetShapes()
    {
        mShapes.clear();

        if (mHealth > 0)
        {
            const float shrinkPerHealth = 30.0f;
            const float maxSize = mBaseRadius * 2.0f;
            const float wallThickness = 30.0f;

            float outerSize = maxSize - (shrinkPerHealth * (4 - mHealth));

            // Ensure outer size doesn't get too small
            float heartDiameter = mBaseRadius * 0.6f;
            if (outerSize < heartDiameter + 2.0f * wallThickness)
            {
                outerSize = heartDiameter + 2.0f * wallThickness;
            }

            float innerSize = outerSize - 2.0f * wallThickness;

            float halfOuter = outerSize / 2.0f;
            float halfInner = innerSize / 2.0f;

            // Top wall
            Rect topWall(mPosition.x - halfOuter, mPosition.y - halfOuter, outerSize, wallThickness);
            mShapes.emplace_back(topWall);

            // Bottom wall
            Rect bottomWall(mPosition.x - halfOuter, mPosition.y + halfInner, outerSize, wallThickness);
            mShapes.emplace_back(bottomWall);

            // Left wall
            Rect leftWall(mPosition.x - halfOuter, mPosition.y - halfOuter, wallThickness, outerSize);
            mShapes.emplace_back(leftWall);

            // Right wall
            Rect rightWall(mPosition.x + halfInner, mPosition.y - halfOuter, wallThickness, outerSize);
            mShapes.emplace_back(rightWall);
        }
        else
        {
            // At 0 health, the base is just a heart-shaped circle
            float radius = mBaseRadius * 0.25f;
            mShapes.emplace_back(Circle(mPosition, radius));
        }

        return mShapes;
    }

    bool Base::IsAlive()
    {
        return mIsAlive;
    }

    void Base::UpdateBounds()
    {
        // Bounding box shrinks by constant amount per health lost
        const float shrinkPerHealth = 40.0f;
        const float maxSize = mBaseRadius * 2.0f;

        float boundingSize;
        if (mHealth > 0)
        {
            // Shrink square based on health
            boundingSize = maxSize - (shrinkPerHealth * (4 - mHealth));

            // Minimum size to cover the heart
            float minSize = mBaseRadius * 0.6f;
            if (boundingSize < minSize)
                boundingSize = minSize;
        }
        else
        {
            // At health 0, just cover the heart circle
            boundingSize = mBaseRadius * 0.5f;
        }

        float halfSize = boundingSize / 2.0f;
        mBounds = CMPUT350::Rect(
            mPosition.x - halfSize,
            mPosition.y - halfSize,
            boundingSize,
            boundingSize);
    }

}