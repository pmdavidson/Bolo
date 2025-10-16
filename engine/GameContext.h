#ifndef GAMECONTEXT_H
#define GAMECONTEXT_H

#include "EngineView.h"
#include "DrawContext.h"
#include <memory>

namespace CMPUT350
{
    class GameObject;
    class NotificationManager;

    class GameContext{
    public:
        // EngineView *EngineContext = nullptr;
        // DrawContext *ScreenContext = nullptr;
        bool isGameOver = false;
        EngineView *EngineContext;
        DrawContext *ScreenContext;
        DrawContext *GUIContext;
        std::weak_ptr<GameObject> CurrObject;
        NotificationManager *NotificationContext;
        
        float DeltaTime = 1.0f / 30.0f; // default to 30 FPS for bullet lifetime
    };
}

#endif // GAMECONTEXT_H
