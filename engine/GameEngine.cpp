#include "GameEngine.h"
#include "FontData.h"
#include "GameObject.h"
#include <memory>    //for std::shared_ptr
#include <stdexcept> //for std::runtime_error
#include <iostream>  //for debug output
#include <algorithm> //for std::remove_if
#include "CollisionObject.h"
#include "CollisionObject.cpp"

/// @brief
namespace CMPUT350
{
    GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string &title)
        : mWindow(sf::VideoMode({width, height}), title),
          mDebugText(mFont, sf::String(""), 14u),
          mDrawContext(std::shared_ptr<sf::RenderWindow>(&mWindow, [](sf::RenderWindow *) {}),
                       std::shared_ptr<sf::Font>(&mFont, [](sf::Font *) {})),
          mGUIContext(std::shared_ptr<sf::RenderWindow>(&mWindow, [](sf::RenderWindow *) {}),
                      std::shared_ptr<sf::Font>(&mFont, [](sf::Font *) {})) //setup GUI context
    {
        //initialize shared engine context
        mContext.EngineContext = this;
        mContext.ScreenContext = &mDrawContext;
        mContext.GUIContext = &mGUIContext;
        mContext.NotificationContext = &mNotificationManager;

        mWindow.setFramerateLimit(30); //limit FPS to reduce CPU usage

        //load embedded font
        if (!mFont.openFromMemory(
                _System_Library_Fonts_Supplemental_Trattatello_ttf,
                sizeof(_System_Library_Fonts_Supplemental_Trattatello_ttf)))
        {
            throw std::runtime_error("Failed to load font from memory!");
        }

        //setup debug text appearance
        mDebugText.setFont(mFont);
        mDebugText.setCharacterSize(14);
        mDebugText.setFillColor(sf::Color::White);
        mDebugText.setPosition(sf::Vector2f(10.f, 10.f));

        //clear object containers
        mGameObjects.clear();
        mPendingObjects.clear();
    }

    GameEngine::~GameEngine()
    {
        //cleanup game state
        mGameObjects.clear();
        mPendingObjects.clear();
        
        //reset notifications
        mNotificationManager = NotificationManager();
        
        //close window if still open
        if (mWindow.isOpen())
        {
            mWindow.close();
        }
    }

    void GameEngine::AddGameObject(std::shared_ptr<GameObject> gameObject)
    {
        //defer adding object until next frame
        mPendingObjects.push_back(std::move(gameObject));
    }

    void GameEngine::Run()
    {
        //main game loop
        while (mWindow.isOpen())
        {
            //remove dead objects
            mGameObjects.erase(
                std::remove_if(mGameObjects.begin(), mGameObjects.end(),
                               [](const std::shared_ptr<GameObject> &obj)
                               { return !obj->IsAlive(); }),
                mGameObjects.end());

            //add new pending objects
            if (!mPendingObjects.empty())
            {
                mGameObjects.insert(mGameObjects.end(),
                                    std::make_move_iterator(mPendingObjects.begin()),
                                    std::make_move_iterator(mPendingObjects.end()));
                mPendingObjects.clear();
            }

            //handle input events
            while (auto event = mWindow.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    mWindow.close();
                }
                else if (auto key = event->getIf<sf::Event::KeyPressed>())
                {
                    char keyChar = '\0';

                    //map key codes to characters
                    switch (key->code)
                    {
                    case sf::Keyboard::Key::W: keyChar = 'w'; break;
                    case sf::Keyboard::Key::A: keyChar = 'a'; break;
                    case sf::Keyboard::Key::S: keyChar = 's'; break;
                    case sf::Keyboard::Key::D: keyChar = 'd'; break;
                    case sf::Keyboard::Key::X: keyChar = 'x'; break;
                    case sf::Keyboard::Key::Num1: keyChar = '1'; break;
                    case sf::Keyboard::Key::Num2: keyChar = '2'; break;
                    case sf::Keyboard::Key::Num3: keyChar = '3'; break;
                    case sf::Keyboard::Key::Num4: keyChar = '4'; break;
                    case sf::Keyboard::Key::Num5: keyChar = '5'; break;
                    case sf::Keyboard::Key::Space: keyChar = ' '; break;
                    default: break;
                    }

                    //dispatch key event to all objects
                    if (keyChar != '\0')
                    {
                        for (auto &obj : mGameObjects)
                        {
                            obj->HandleKeyEvent(&mContext, keyChar);
                        }
                    }
                }
            }

            //update all objects
            for (auto &obj : mGameObjects)
            {
                mContext.CurrObject = obj;
                obj->Update(&mContext);
            }

            //prepare for collision detection
            std::vector<std::shared_ptr<CollisionObject>> staticObjects;
            std::vector<std::shared_ptr<CollisionObject>> dynamicObjects;

            //split into static and dynamic sets
            for (auto& obj : mGameObjects)
            {
                std::shared_ptr<CollisionObject> collisionObj = std::dynamic_pointer_cast<CollisionObject>(obj);
                if (collisionObj == nullptr)
                    continue;
                    
                if (collisionObj->IsStatic())
                    staticObjects.push_back(collisionObj);
                else
                    dynamicObjects.push_back(collisionObj);
            }

            //dynamic vs dynamic collisions
            for (size_t i = 0; i < dynamicObjects.size(); ++i)
            {
                for (size_t j = i + 1; j < dynamicObjects.size(); ++j)
                {
                    auto& objA = dynamicObjects[i];
                    auto& objB = dynamicObjects[j];

                    if (objA->GetBounds().intersects(objB->GetBounds()))
                    {
                        Point2D collisionPoint;
                        if (objA->CollidesWith(objB, &collisionPoint)) {
                            objA->CollisionEnter(objB, collisionPoint);
                            objB->CollisionEnter(objA, collisionPoint);
                        }
                    }
                }
            }

            //dynamic vs static collisions
            for (auto& dynamicObj : dynamicObjects)
            {
                for (auto& staticObj : staticObjects)
                {
                    if (dynamicObj->GetBounds().intersects(staticObj->GetBounds()))
                    {
                        Point2D collisionPoint;
                        if (dynamicObj->CollidesWith(staticObj, &collisionPoint)) {
                            dynamicObj->CollisionEnter(staticObj, collisionPoint);
                            staticObj->CollisionEnter(dynamicObj, collisionPoint);
                        }
                    }
                }
            }

            //it was this version but with high density it became very laggy
            // Process collisions
            // for (size_t i = 0; i < mGameObjects.size(); ++i)
            // {
            //     std::shared_ptr<CollisionObject> objA = std::dynamic_pointer_cast<CollisionObject>(mGameObjects[i]);
            //     if (objA == nullptr)
            //         continue;

            //     for (size_t j = i + 1; j < mGameObjects.size(); ++j)
            //     {
            //         std::shared_ptr<CollisionObject> objB = std::dynamic_pointer_cast<CollisionObject>(mGameObjects[j]);
            //         if (objB == nullptr)
            //             continue;

            //         if (objB->IsStatic() && objA->IsStatic())
            //             continue; // Skip static-static

            //         if (!objA->GetBounds().intersects(objB->GetBounds()))
            //         {
            //             continue;
            //         }

            //         Point2D collisionPoint;
            //         if (objA->CollidesWith(objB, &collisionPoint)) {
            //             objA->CollisionEnter(objB, collisionPoint);
            //             objB->CollisionEnter(objA, collisionPoint);
            //         }
            //         }
            //     }

            //late update phase
            for (auto &obj : mGameObjects)
            {
                mContext.CurrObject = obj;
                obj->LateUpdate(&mContext);
            }

            //render scene
            mWindow.clear();
            for (auto &obj : mGameObjects)
            {
                mContext.CurrObject = obj;
                obj->RenderBackground(&mContext);
            }
            for (auto &obj : mGameObjects)
            {
                mContext.CurrObject = obj;
                obj->RenderForeground(&mContext);
            }
            mWindow.draw(mDebugText);
            mWindow.display();
        }
    }
}
