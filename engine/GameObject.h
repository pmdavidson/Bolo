#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <string>
#include "GameContext.h"

// engine/GameObject.h
#pragma once

namespace CMPUT350
{

    class GameObject
    {
    public:
        virtual ~GameObject();

        virtual void Update(GameContext *context) { mLastContext = context; }
        virtual void LateUpdate(GameContext *context) { mLastContext = context; }

        virtual void RenderBackground(GameContext *context) {}
        virtual void RenderForeground(GameContext *context) {}

        virtual bool HandleKeyEvent(GameContext *context, char key) { return false; }
        virtual bool IsAlive() { return mIsAlive; }

        virtual void Kill() { mIsAlive = false; } // change everything to kill if they die
        virtual void ReceiveNotification(const std::string &message);

    protected:
        bool mIsAlive = true;
        GameContext *mLastContext = nullptr; // cached pointer
    };

}

#endif // GAMEOBJECT_H