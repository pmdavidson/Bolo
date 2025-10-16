#include <cassert>
#include "../engine/MathUtil.h"
#include "Player.h"
#include "Bullet.h"
#include "Explosion.h"
#include <assert.h>

namespace CMPUT350
{

    Player::Player(CMPUT350::Point2D loc) : mPosition(loc),
                                            mDirection({0, -1}),
                                            mTurretDirection({0, -1}),
                                            mSpeed(0),
                                            mTankSize(30.f),
                                            mTurretLength(18.f)
    {
        UpdateBounds();
    }

    void Player::Update(GameContext *context)
    {
        if (!mIsAlive)
            return;

        GameObject::Update(context);

        // Handle movement
        UpdateMovement();

        // Update bounding box
        UpdateBounds();
    }

    void Player::LateUpdate(GameContext *context)
    {
        GameObject::LateUpdate(context);

        // Decrement fire cooldown at end of frame
        if (mFireCooldown > 0)
        {
            --mFireCooldown;
        }

        // Spawn explosion when player dies
        if (!mIsAlive)
        {
            // Checks if there is an existing context that corresponds to the engine
            if (mLastContext && mLastContext->EngineContext)
            {
                // Queue explosion at this location
                auto explosion = std::make_shared<Explosion>(mPosition, 20);
                mLastContext->EngineContext->AddGameObject(explosion);
            }
            // Clear context reference to prevent dangling pointer
            mLastContext = nullptr;
        }
    }

    void Player::RenderBackground(GameContext *context)
    {
        if (!mIsAlive)
            return;

        float trackWidth = 6.0f;
        float bodyWidth = 12.0f;
        float trackLength = mTankSize * 0.8f;

        // Perpendicular vector (left-right relative to tank direction)
        CMPUT350::Point2D perp(-mDirection.y, mDirection.x);

        // Distance between tracks
        float trackSeparation = mTankSize * 0.6f;
        float bodyLength = trackSeparation * 1.4f;

        // Draw H-shape: tracks are vertical bars, body is horizontal crossbar

        // Left track (vertical part of H)
        CMPUT350::Point2D leftTrackCenter = mPosition + perp * (trackSeparation / 2.0f);
        CMPUT350::Point2D leftStart = leftTrackCenter - mDirection * (trackLength / 2.0f);
        CMPUT350::Point2D leftEnd = leftTrackCenter + mDirection * (trackLength / 2.0f);
        context->ScreenContext->DrawLine(leftStart, leftEnd, trackWidth, CMPUT350::RGBColor(0, 255, 0)); // Green

        // Right track (vertical part of H)
        CMPUT350::Point2D rightTrackCenter = mPosition - perp * (trackSeparation / 2.0f);
        CMPUT350::Point2D rightStart = rightTrackCenter - mDirection * (trackLength / 2.0f);
        CMPUT350::Point2D rightEnd = rightTrackCenter + mDirection * (trackLength / 2.0f);
        context->ScreenContext->DrawLine(rightStart, rightEnd, trackWidth, CMPUT350::RGBColor(0, 255, 0));

        // Main body (horizontal crossbar of H)
        CMPUT350::Point2D bodyStart = mPosition - perp * (bodyLength / 2.0f);
        CMPUT350::Point2D bodyEnd = mPosition + perp * (bodyLength / 2.0f);
        context->ScreenContext->DrawLine(bodyStart, bodyEnd, bodyWidth, CMPUT350::RGBColor(0, 255, 0));

        // Draw turret (on top of body)
        float turretWidth = 6.0f;
        float turretLength = mTurretLength;

        CMPUT350::Point2D turretDir = mTurretDirection;
        float currentLength = std::sqrt(turretDir.x * turretDir.x + turretDir.y * turretDir.y);
        if (currentLength > 0)
        {
            turretDir.x = (turretDir.x / currentLength) * turretLength;
            turretDir.y = (turretDir.y / currentLength) * turretLength;
        }

        CMPUT350::Point2D turretEnd = mPosition + turretDir;
        context->ScreenContext->DrawLine(mPosition, turretEnd, turretWidth, CMPUT350::RGBColor(255, 255, 255));
    }

    void Player::RenderForeground(GameContext *context)
    {
    }

    bool Player::HandleKeyEvent(CMPUT350::GameContext *context, char key)
    {
        const float SPEED_STEP = 5.0f;                        // Pixels per frame per key press
        const float SPEED_MAX = 15.0f;                        // Clamp to ±15 pixels/frame
        const float ROTATE_ANGLE = CMPUT350::DegToRad(45.0f); // 45° per press

        assert(mIsAlive == true);

        switch (key)
        {
        // Speed control
        case 'w':
            mSpeed = std::min(mSpeed + SPEED_STEP, SPEED_MAX); // Ensures max speed is 15f
            break;

        case 'x':
            mSpeed = std::max(mSpeed - SPEED_STEP, -SPEED_MAX); // Ensures max speed is 15f
            break;

        case 's':
            mSpeed = 0.f; // Stop
            break;

        // Tank rotation
        case 'a':
            mDirection = CMPUT350::Rotate(mDirection, -ROTATE_ANGLE); // Counterclockwise
            mTurretDirection = CMPUT350::Rotate(mTurretDirection, -ROTATE_ANGLE);
            break;

        case 'd':
            mDirection = CMPUT350::Rotate(mDirection, ROTATE_ANGLE); // Clockwise
            mTurretDirection = CMPUT350::Rotate(mTurretDirection, ROTATE_ANGLE);
            break;

        // Turret rotation
        case '1':
            mTurretDirection = CMPUT350::Rotate(mTurretDirection, -ROTATE_ANGLE); // Counterclockwise
            break;

        case '2':
            mTurretDirection = CMPUT350::Rotate(mTurretDirection, ROTATE_ANGLE); // Clockwise
            break;

        // Fire bullet
        case ' ':
            if (mFireCooldown == 0)
            {
                FireBullet(context);
            }
            break;

        default:
            return false; // key not handled
        }

        return true; // key handled
    }

    void Player::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
    {
        // Use current position as approximate collision point
        CollisionEnter(obj, mPosition);
    }

    void Player::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
    {
        if (!mIsAlive)
            return;

        // Ignore own bullet
        if (mLastBullet.lock().get() == obj.get())
            return;

        // Get their shapes and ours
        const auto &theirShapes = obj->GetShapes();
        const auto &myShapes = this->GetShapes();

        for (const Shape &mine : myShapes)
        {
            for (const Shape &theirs : theirShapes)
            {
                Point2D hit;
                if (ShapeIntersect(mine, theirs, &hit))
                {
                    // Kill the player on confirmed collision
                    mIsAlive = false;
                    return;
                }
            }
        }
    }

    const Rect &Player::GetBounds()
    {
        return mBounds;
    }

    const std::vector<Shape> &Player::GetShapes()
    {
        mShapes.clear();

        float trackLength = mTankSize * 0.8f;
        CMPUT350::Point2D perp(-mDirection.y, mDirection.x);
        float trackSeparation = mTankSize * 0.6f;
        float bodyLength = trackSeparation * 1.4f;

        // Left track
        CMPUT350::Point2D leftTrackCenter = mPosition + perp * (trackSeparation / 2.0f);
        CMPUT350::Point2D leftStart = leftTrackCenter - mDirection * (trackLength / 2.0f);
        CMPUT350::Point2D leftEnd = leftTrackCenter + mDirection * (trackLength / 2.0f);
        mShapes.emplace_back(mBounds);

        // Right track
        CMPUT350::Point2D rightTrackCenter = mPosition - perp * (trackSeparation / 2.0f);
        CMPUT350::Point2D rightStart = rightTrackCenter - mDirection * (trackLength / 2.0f);
        CMPUT350::Point2D rightEnd = rightTrackCenter + mDirection * (trackLength / 2.0f);
        mShapes.emplace_back(Shape(Line(rightStart, rightEnd)));

        // Main body (horizontal crossbar)
        CMPUT350::Point2D bodyStart = mPosition - perp * (bodyLength / 2.0f);
        CMPUT350::Point2D bodyEnd = mPosition + perp * (bodyLength / 2.0f);
        mShapes.emplace_back(Shape(Line(bodyStart, bodyEnd)));

        // Turret
        float turretLength = mTurretLength;
        CMPUT350::Point2D turretDir = mTurretDirection;
        float currentLength = std::sqrt(turretDir.x * turretDir.x + turretDir.y * turretDir.y);
        if (currentLength > 0)
        {
            turretDir.x = (turretDir.x / currentLength) * turretLength;
            turretDir.y = (turretDir.y / currentLength) * turretLength;
        }
        CMPUT350::Point2D turretEnd = mPosition + turretDir;
        mShapes.push_back(Shape(Line(mPosition, turretEnd)));

        return mShapes;
    }

    bool Player::IsAlive()
    {
        return mIsAlive;
    }

    void Player::UpdateMovement()
    {
        // Move tank in the facing direction by mSpeed pixels per frame
        mPosition.x += mDirection.x * mSpeed;
        mPosition.y += mDirection.y * mSpeed;
    }

    void Player::FireBullet(GameContext *context)
    {
        if (!mIsAlive)
            return;

        const float MIN_BULLET_SPEED = 2.0f * 15.0f;

        CMPUT350::Point2D bulletDir = mTurretDirection;
        bulletDir.Normalize();

        CMPUT350::Point2D bulletVelocity = bulletDir * MIN_BULLET_SPEED;

        CMPUT350::Point2D tankVelocity = mDirection * mSpeed;
        bulletVelocity += tankVelocity; // Add tank velocity to bullet

        auto bullet = std::make_shared<Bullet>(this, mPosition, bulletVelocity, 3.0f);

        // Track the most recent bullet with weak_ptr
        mLastBullet = bullet;

        context->EngineContext->AddGameObject(bullet);

        // Set cooldown to 5 frames
        mFireCooldown = 5;
    }

    void Player::UpdateBounds()
    {
        // Start with the tank body bounding box
        mBounds = CMPUT350::Rect(
            mPosition.x - mTankSize / 2,
            mPosition.y - mTankSize / 2,
            mTankSize,
            mTankSize);

        // Calculate turret tip position
        CMPUT350::Point2D turretDir = mTurretDirection;
        float currentLength = std::sqrt(turretDir.x * turretDir.x + turretDir.y * turretDir.y);

        assert(currentLength > 0);

        turretDir.x = (turretDir.x / currentLength) * mTurretLength;
        turretDir.y = (turretDir.y / currentLength) * mTurretLength;

        CMPUT350::Point2D turretTip = mPosition + turretDir;

        // Expand bounding box to include turret tip
        mBounds |= turretTip;
    }
}