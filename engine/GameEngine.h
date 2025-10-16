#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <memory>
#include <vector>
#include <string>
#include <cstdlib>

#include "MathUtil.h"
#include "GameObject.h"
#include "CollisionObject.h"
#include "DrawContext.h"
#include "EngineView.h"
#include "GameContext.h"
#include "NotificationManager.h"

namespace CMPUT350
{
    class GameEngine : public EngineView
    {
    public:
        GameEngine(unsigned int width, unsigned int height, const std::string &name);
        ~GameEngine();

        void AddGameObject(std::shared_ptr<GameObject> gameObject) override;
        void Run();

        const_iterator cbegin() const override { return mGameObjects.cbegin(); }
        const_iterator cend() const override { return mGameObjects.cend(); }
        const_iterator begin() const override { return mGameObjects.begin(); }
        const_iterator end() const override { return mGameObjects.end(); }

        bool GetCollisionPoint(
            const std::shared_ptr<CollisionObject> &a,
            const std::shared_ptr<CollisionObject> &b,
            Point2D &outPoint);

    private:
        sf::RenderWindow mWindow;
        sf::Font mFont;
        sf::Text mDebugText;

        tGameObjectVec mGameObjects;
        tGameObjectVec mPendingObjects;

        DrawContext mDrawContext; // For gameplay
        DrawContext mGUIContext;  // For GUI/HUD
        NotificationManager mNotificationManager;

        GameContext mContext;
    };
}

#endif // GAMEENGINE_H
