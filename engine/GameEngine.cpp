#include "GameEngine.h"
#include "FontData.h"
#include "GameObject.h"
#include <memory>    // for std::shared_ptr
#include <stdexcept> // for std::runtime_error
#include <iostream>  // for debug output
#include <algorithm> // for std::remove_if
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
                      std::shared_ptr<sf::Font>(&mFont, [](sf::Font *) {})) // NEW: GUI context setup
    {
        // Initialize context pointers
        mContext.EngineContext = this;
        mContext.ScreenContext = &mDrawContext;
        mContext.GUIContext = &mGUIContext;
        mContext.NotificationContext = &mNotificationManager;

        // Limit to 30 frames per second
        mWindow.setFramerateLimit(30);

        // Load font from memory
        if (!mFont.openFromMemory(
                _System_Library_Fonts_Supplemental_Trattatello_ttf,
                sizeof(_System_Library_Fonts_Supplemental_Trattatello_ttf)))
        {
            throw std::runtime_error("Failed to load font from memory!");
        }

        // Debug text setup
        mDebugText.setFont(mFont);
        mDebugText.setCharacterSize(14);
        mDebugText.setFillColor(sf::Color::White);
        mDebugText.setPosition(sf::Vector2f(10.f, 10.f));

        // Clear object containers
        mGameObjects.clear();
        mPendingObjects.clear();
    }

    GameEngine::~GameEngine()
    {
        mGameObjects.clear();
        mPendingObjects.clear();

        // Clear notification manager
        mNotificationManager = NotificationManager();

        // Close window if still open
        if (mWindow.isOpen())
        {
            mWindow.close();
        }
    }

    void GameEngine::AddGameObject(std::shared_ptr<GameObject> gameObject)
    {
        mPendingObjects.push_back(std::move(gameObject));
    }

    void GameEngine::Run()
    {
        // Main game loop
        while (mWindow.isOpen())
        {

            // Iterate through mGameObjects and remove dead ones
            mGameObjects.erase(
                std::remove_if(mGameObjects.begin(), mGameObjects.end(),
                               [](const std::shared_ptr<GameObject> &obj)
                               { return !obj->IsAlive(); }),
                mGameObjects.end());

            // Add pending objects
            if (!mPendingObjects.empty())
            {
                mGameObjects.insert(mGameObjects.end(),
                                    std::make_move_iterator(mPendingObjects.begin()),
                                    std::make_move_iterator(mPendingObjects.end()));
                mPendingObjects.clear();
            }

            // Process events
            while (auto event = mWindow.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    mWindow.close();
                }
                else if (auto key = event->getIf<sf::Event::KeyPressed>())
                {
                    char keyChar = '\0';

                    // Map SFML key to char
                    switch (key->code)
                    {
                    case sf::Keyboard::Key::W:
                        keyChar = 'w';
                        break;
                    case sf::Keyboard::Key::A:
                        keyChar = 'a';
                        break;
                    case sf::Keyboard::Key::S:
                        keyChar = 's';
                        break;
                    case sf::Keyboard::Key::D:
                        keyChar = 'd';
                        break;
                    case sf::Keyboard::Key::X:
                        keyChar = 'x';
                        break;
                    case sf::Keyboard::Key::Num1:
                        keyChar = '1';
                        break;
                    case sf::Keyboard::Key::Num2:
                        keyChar = '2';
                        break;
                    case sf::Keyboard::Key::Num3:
                        keyChar = '3';
                        break;
                    case sf::Keyboard::Key::Num4:
                        keyChar = '4';
                        break;
                    case sf::Keyboard::Key::Num5:
                        keyChar = '5';
                        break;
                    case sf::Keyboard::Key::Space:
                        keyChar = ' ';
                        break;
                    default:
                        break; // Key not handled
                    }

                    if (keyChar != '\0')
                    {
                        for (auto &obj : mGameObjects)
                        {
                            obj->HandleKeyEvent(&mContext, keyChar);
                        }
                    }
                }
            }

            // Update
            for (auto &obj : mGameObjects)
            {
                mContext.CurrObject = obj;
                obj->Update(&mContext);
            }

            // Process collisions with aggressive optimization for high wall counts
            // Separate static and dynamic objects for O(n) static checks
            std::vector<std::shared_ptr<CollisionObject>> staticObjects;
            std::vector<std::shared_ptr<CollisionObject>> dynamicObjects;

            // Categorize objects once per frame
            for (auto &obj : mGameObjects)
            {
                std::shared_ptr<CollisionObject> collisionObj = std::dynamic_pointer_cast<CollisionObject>(obj);
                if (collisionObj == nullptr)
                    continue;

                if (collisionObj->IsStatic())
                    staticObjects.push_back(collisionObj);
                else
                    dynamicObjects.push_back(collisionObj);
            }

            // Check dynamic vs dynamic collisions
            for (size_t i = 0; i < dynamicObjects.size(); ++i)
            {
                for (size_t j = i + 1; j < dynamicObjects.size(); ++j)
                {
                    auto &objA = dynamicObjects[i];
                    auto &objB = dynamicObjects[j];

                    if (objA->GetBounds().intersects(objB->GetBounds()))
                    {
                        Point2D collisionPoint;
                        if (objA->CollidesWith(objB, &collisionPoint))
                        {
                            objA->CollisionEnter(objB, collisionPoint);
                            objB->CollisionEnter(objA, collisionPoint);
                        }
                    }
                }
            }

            // Check dynamic vs static collisions (only dynamic objects can hit static)
            for (auto &dynamicObj : dynamicObjects)
            {
                for (auto &staticObj : staticObjects)
                {
                    if (dynamicObj->GetBounds().intersects(staticObj->GetBounds()))
                    {
                        Point2D collisionPoint;
                        if (dynamicObj->CollidesWith(staticObj, &collisionPoint))
                        {
                            dynamicObj->CollisionEnter(staticObj, collisionPoint);
                            staticObj->CollisionEnter(dynamicObj, collisionPoint);
                        }
                    }
                }
            }

            // Late update
            for (auto &obj : mGameObjects)
            {
                mContext.CurrObject = obj;
                obj->LateUpdate(&mContext);
            }

            // Render
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