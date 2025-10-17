#include "Bolo.h"
#include "Wall.h"
#include "Enemy.h"
#include "Base.h"
#include "Bullet.h"
#include "Explosion.h"
#include "../engine/DrawContext.h"
#include "../engine/GameContext.h"
#include "../engine/NotificationManager.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <stack>

namespace CMPUT350
{
    class NotificationManager;

    Bolo::Bolo(int mazeWidth, int mazeHeight, int cellSize, int initialLevel)
        : mState(GameState::SelectingLevel),
          mLevel(initialLevel),
          mScore(0),
          mMazeDensity(3),
          mStateTimer(0),
          mMazeWidth(mazeWidth),
          mMazeHeight(mazeHeight),
          mCellSize(cellSize),
          mWorldSize(mazeWidth * cellSize),
          mGameplayWidth(1024),
          mGuiXStart(1024),
          mBounds(0, 0, mazeWidth * cellSize, mazeWidth * cellSize),
          mWalls(mazeHeight, std::vector<std::vector<bool>>(mazeWidth, std::vector<bool>(2))),
          mCellExplored(mazeHeight, std::vector<bool>(mazeWidth))
    {
        // Initialize random seed
        static bool seeded = false;
        if (!seeded)
        {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            seeded = true;
        }

        // Clear maze data
        ClearMaze();

        // Generate initial maze for preview
        GenerateMaze();
    }

    void Bolo::Update(GameContext *context)
    {
        GameObject::Update(context);

        switch (mState)
        {
        case GameState::SelectingLevel:
            // Waiting for player to press 1-5 to select level
            break;

        case GameState::SelectingDensity:
            // Waiting for player to press 1-5 to select density
            break;

        case GameState::InitializingLevel:
            // Wait for engine to clean up dead objects from previous level
            if (mStateTimer > 0)
            {
                mStateTimer--;
                break;
            }

            // Now generate fresh maze and spawn all objects
            GenerateMaze();
            SpawnPlayer(context);
            SpawnBases(context);
            AddWallsToEngine(context);

            if (context->NotificationContext && !mIsRegistered)
            {
                auto self = shared_from_this();
                // NotificationContext is a NotificationManager* member of GameContext
                CMPUT350::NotificationManager *notificationMgr = context->NotificationContext;
                notificationMgr->Register(self, "BASE_DESTROYED");
                notificationMgr->Register(self, "ENEMY_DESTROYED");
                mIsRegistered = true;
            }

            mState = GameState::GamePlay;
            mStateTimer = 0;
            break;

        case GameState::GamePlay:
            // Check if all bases destroyed
            if (AllBasesDestroyed())
            {
                mState = GameState::EndLevel;
                mStateTimer = 90; // 3 seconds
            }
            break;

        case GameState::EndLevel:
            if (mStateTimer > 0)
            {
                mStateTimer--;
            }
            else
            {
                // Kill all remaining level objects (enemies, bullets, explosions, player)
                CleanupLevel(context);

                if (mLevel < 5)
                    mLevel++;

                mState = GameState::CleaningUp;
                mStateTimer = 30;
            }
            break;

        case GameState::CleaningUp:
            if (mStateTimer > 0)
            {
                mStateTimer--;
            }
            else
            {
                // Now spawn new level
                mState = GameState::InitializingLevel;
                mStateTimer = 5; // Small delay before spawning
            }
            break;
        }
    }

    void Bolo::LateUpdate(GameContext *context)
    {
        GameObject::LateUpdate(context);
    }

    void Bolo::RenderBackground(GameContext *context)
    {
        // Set viewport to gameplay area only (prevents rendering over GUI)
        if (mState == GameState::GamePlay || mState == GameState::EndLevel)
        {
            context->ScreenContext->SetGameplayViewport();

            if (mState == GameState::EndLevel)
            {
                // EndLevel: center on map and zoom out to show entire world
                float worldCenter = mWorldSize / 2.0f;
                // Fit entire square world into 1024x1024 gameplay view: size equals world bounds
                context->ScreenContext->SetGameplayCenterAndSize(
                    Point2D(worldCenter, worldCenter),
                    sf::Vector2f(static_cast<float>(mWorldSize), static_cast<float>(mWorldSize)));
            }
            else if (mPlayer)
            {
                // Gameplay: follow player
                float playerX = mPlayer->GetBounds().topLeft.x + mPlayer->GetBounds().width / 2.0f;
                float playerY = mPlayer->GetBounds().topLeft.y + mPlayer->GetBounds().height / 2.0f;
                context->ScreenContext->SetContextCenter(Point2D(playerX, playerY));
            }
        }
        // Background rendering handled by walls
    }

    void Bolo::RenderForeground(GameContext *context)
    {
        context->ScreenContext->SetFullViewport();

        RenderGUI(context);

        // Render state-specific text and viewport
        if (mState == GameState::SelectingLevel)
        {
            // Display instructions for level selection
            context->ScreenContext->DrawText(
                "Select Level (1-5)",
                24,
                Point2D(mGuiXStart + 20, 380),
                RGBColor(255, 255, 0));
            context->ScreenContext->DrawText(
                "Level determines which",
                18,
                Point2D(mGuiXStart + 20, 420),
                RGBColor(255, 255, 255));
            context->ScreenContext->DrawText(
                "enemies may spawn",
                18,
                Point2D(mGuiXStart + 20, 445),
                RGBColor(255, 255, 255));
        }
        else if (mState == GameState::SelectingDensity)
        {
            // Display instructions for density selection
            context->ScreenContext->DrawText(
                "Level: " + std::to_string(mLevel),
                20,
                Point2D(mGuiXStart + 20, 350),
                RGBColor(0, 255, 0));

            context->ScreenContext->DrawText(
                "Select Density (1-5)",
                24,
                Point2D(mGuiXStart + 20, 400),
                RGBColor(255, 255, 0));
            context->ScreenContext->DrawText(
                "1 = Sparse maze",
                18,
                Point2D(mGuiXStart + 20, 440),
                RGBColor(255, 255, 255));
            context->ScreenContext->DrawText(
                "5 = Dense maze",
                18,
                Point2D(mGuiXStart + 20, 465),
                RGBColor(255, 255, 255));
        }
        else if (mState == GameState::GamePlay)
        {
            // Set gameplay viewport and follow player
            context->ScreenContext->SetGameplayViewport();

            if (mPlayer)
            {
                float playerX = mPlayer->GetBounds().topLeft.x + mPlayer->GetBounds().width / 2.0f;
                float playerY = mPlayer->GetBounds().topLeft.y + mPlayer->GetBounds().height / 2.0f;
                context->ScreenContext->SetContextCenter(Point2D(playerX, playerY));
            }
        }
        else if (mState == GameState::EndLevel)
        {
            // Display congratulations screen (centered in gameplay area)
            float centerX = mGameplayWidth / 2.0f;
            float centerY = 512.0f;

            context->ScreenContext->DrawText(
                "LEVEL COMPLETE!",
                40,
                Point2D(centerX - 200, centerY - 100),
                RGBColor(0, 255, 0));

            context->ScreenContext->DrawText(
                "Level " + std::to_string(mLevel) + " Finished",
                24,
                Point2D(centerX - 120, centerY - 40),
                RGBColor(255, 255, 255));

            context->ScreenContext->DrawText(
                "Score: " + std::to_string(mScore),
                24,
                Point2D(centerX - 80, centerY + 20),
                RGBColor(255, 255, 0));

            int secondsLeft = (mStateTimer + 29) / 30;
            context->ScreenContext->DrawText(
                "Next level in " + std::to_string(secondsLeft) + " seconds...",
                18,
                Point2D(centerX - 150, centerY + 80),
                RGBColor(200, 200, 200));

            // Set gameplay viewport and zoom out to show entire world
            context->ScreenContext->SetGameplayViewport();

            float worldCenter = mWorldSize / 2.0f;
            context->ScreenContext->SetGameplayCenterAndSize(
                Point2D(worldCenter, worldCenter),
                sf::Vector2f(static_cast<float>(mWorldSize / 0.99f), static_cast<float>(mWorldSize / 0.99f)));
        }
    }

    bool Bolo::HandleKeyEvent(GameContext *context, char key)
    {
        if (mState == GameState::SelectingLevel)
        {
            // Check for level selection (1-5)
            if (key >= '1' && key <= '5')
            {
                mLevel = key - '0';
                mState = GameState::SelectingDensity;
                return true;
            }
        }
        else if (mState == GameState::SelectingDensity)
        {
            // Check for density selection (1-5)
            if (key >= '1' && key <= '5')
            {
                mMazeDensity = key - '0';
                mState = GameState::InitializingLevel;
                return true;
            }
        }
        return false;
    }

    const Rect &Bolo::GetBounds()
    {
        return mBounds;
    }

    void Bolo::ClearMaze()
    {
        for (int y = 0; y < mMazeHeight; y++)
        {
            for (int x = 0; x < mMazeWidth; x++)
            {
                mWalls[y][x][0] = true; // Right wall
                mWalls[y][x][1] = true; // Down wall
                mCellExplored[y][x] = false;
            }
        }
    }

    void Bolo::GenerateMaze()
    {
        ClearMaze();
        DFS(0, 0);
        RemoveWalls(mMazeDensity);
    }

    void Bolo::DFS(int startX, int startY)
    {
        // Iterative DFS to carve paths through the maze
        std::stack<std::pair<int, int>> cellStack;

        mCellExplored[startY][startX] = true;
        cellStack.push({startX, startY});

        while (!cellStack.empty())
        {
            int x = cellStack.top().first;
            int y = cellStack.top().second;

            // Find all unvisited neighbors (0=right, 1=down, 2=left, 3=up)
            std::vector<int> directions;
            if (x + 1 < mMazeWidth && !mCellExplored[y][x + 1])
                directions.push_back(0);
            if (y + 1 < mMazeHeight && !mCellExplored[y + 1][x])
                directions.push_back(1);
            if (x - 1 >= 0 && !mCellExplored[y][x - 1])
                directions.push_back(2);
            if (y - 1 >= 0 && !mCellExplored[y - 1][x])
                directions.push_back(3);

            if (!directions.empty())
            {
                // Randomly choose a direction and carve path
                int dir = directions[std::rand() % directions.size()];
                int nextX = x, nextY = y;

                // Remove wall and move to next cell
                if (dir == 0)
                {
                    mWalls[y][x][0] = false;
                    nextX++;
                }
                else if (dir == 1)
                {
                    mWalls[y][x][1] = false;
                    nextY++;
                }
                else if (dir == 2)
                {
                    mWalls[y][x - 1][0] = false;
                    nextX--;
                }
                else
                {
                    mWalls[y - 1][x][1] = false;
                    nextY--;
                }

                mCellExplored[nextY][nextX] = true;
                cellStack.push({nextX, nextY});
            }
            else
            {
                cellStack.pop(); // Backtrack
            }
        }
    }

    void Bolo::RemoveWalls(int density)
    {
        // Cap density at 5 to prevent excessive wall density
        if (density > 5)
            density = 5;

        // Calculate target percentage of DFS walls to keep
        // density 1: (5 + 2*1)/18 = 7/18 = 38.8% keep -> remove 61.2%
        // density 5: (5 + 2*5)/18 = 15/18 = 83.3% keep -> remove 16.7%
        float keepPercentage = (5.0f + 2.0f * density) / 18.0f;

        // Count current walls after DFS
        int currentWalls = 0;
        for (int y = 0; y < mMazeHeight; y++)
        {
            for (int x = 0; x < mMazeWidth; x++)
            {
                if (x < mMazeWidth - 1 && mWalls[y][x][0])
                    currentWalls++;
                if (y < mMazeHeight - 1 && mWalls[y][x][1])
                    currentWalls++;
            }
        }

        // Calculate how many DFS walls to keep
        int targetWalls = static_cast<int>(currentWalls * keepPercentage);

        // How many walls
        int wallsToRemove = currentWalls - targetWalls;

        if (wallsToRemove <= 0)
            return;

        // Collect all existing interior walls
        std::vector<std::pair<int, int>> wallList; // Stores (y * mMazeWidth + x) * 2 + dir
        for (int y = 0; y < mMazeHeight; y++)
        {
            for (int x = 0; x < mMazeWidth; x++)
            {
                if (x < mMazeWidth - 1 && mWalls[y][x][0])
                    wallList.push_back({y * mMazeWidth + x, 0});
                if (y < mMazeHeight - 1 && mWalls[y][x][1])
                    wallList.push_back({y * mMazeWidth + x, 1});
            }
        }

        // Shuffle the wall list for random removal
        for (size_t i = wallList.size() - 1; i > 0; i--)
        {
            size_t j = std::rand() % (i + 1);
            std::swap(wallList[i], wallList[j]);
        }

        // Remove random walls up to the target
        for (int i = 0; i < wallsToRemove && i < (int)wallList.size(); i++)
        {
            int cellIndex = wallList[i].first;
            int dir = wallList[i].second;
            int y = cellIndex / mMazeWidth;
            int x = cellIndex % mMazeWidth;
            mWalls[y][x][dir] = false;
        }
    }

    void Bolo::AddWallsToEngine(GameContext *context)
    {
        if (!context->EngineContext)
            return;

        const float wallWidth = 4.0f;

        // Add outer boundary walls (always present)
        // Top wall
        auto topWall = std::make_shared<Wall>(Point2D(0, 0), mWorldSize, kHorizontal, wallWidth);
        context->EngineContext->AddGameObject(topWall);

        // Bottom wall
        auto bottomWall = std::make_shared<Wall>(Point2D(0, mWorldSize), mWorldSize, kHorizontal, wallWidth);
        context->EngineContext->AddGameObject(bottomWall);

        // Left wall
        auto leftWall = std::make_shared<Wall>(Point2D(0, 0), mWorldSize, kVertical, wallWidth);
        context->EngineContext->AddGameObject(leftWall);

        // Right wall
        auto rightWall = std::make_shared<Wall>(Point2D(mWorldSize, 0), mWorldSize, kVertical, wallWidth);
        context->EngineContext->AddGameObject(rightWall);

        // Add interior walls based on maze data
        for (int y = 0; y < mMazeHeight; y++)
        {
            for (int x = 0; x < mMazeWidth; x++)
            {
                float cellX = x * mCellSize;
                float cellY = y * mCellSize;

                // Right wall (vertical)
                if (mWalls[y][x][0] && x < mMazeWidth - 1)
                {
                    auto wall = std::make_shared<Wall>(
                        Point2D(cellX + mCellSize, cellY),
                        mCellSize,
                        kVertical,
                        wallWidth);
                    context->EngineContext->AddGameObject(wall);
                }

                // Down wall (horizontal)
                if (mWalls[y][x][1] && y < mMazeHeight - 1)
                {
                    auto wall = std::make_shared<Wall>(
                        Point2D(cellX, cellY + mCellSize),
                        mCellSize,
                        kHorizontal,
                        wallWidth);
                    context->EngineContext->AddGameObject(wall);
                }
            }
        }
    }

    void Bolo::SpawnPlayer(GameContext *context)
    {
        if (!context->EngineContext)
            return;

        // Spawn player at a random cell
        int cellX = std::rand() % mMazeWidth;
        int cellY = std::rand() % mMazeHeight;

        // Spawn player at center of cell
        float worldX = cellX * mCellSize + mCellSize / 2.0f;
        float worldY = cellY * mCellSize + mCellSize / 2.0f;

        // Create new player (old one should already be killed in CleanupLevel)
        mPlayer = std::make_shared<Player>(Point2D(worldX, worldY));
        context->EngineContext->AddGameObject(mPlayer);
    }

    void Bolo::SpawnBases(GameContext *context)
    {
        if (!context->EngineContext)
            return;

        // Spawn 6 bases at random unoccupied locations
        int basesSpawned = 0;
        int attempts = 0;

        while (basesSpawned < 6 && attempts < 1000)
        {
            int cellX = std::rand() % mMazeWidth;
            int cellY = std::rand() % mMazeHeight;

            // Spawn base at center of cell
            float worldX = cellX * mCellSize + mCellSize / 2.0f;
            float worldY = cellY * mCellSize + mCellSize / 2.0f;

            // Pass the current game level to the base so it spawns appropriate enemies
            auto base = std::make_shared<Base>(Point2D(worldX, worldY), mLevel);
            context->EngineContext->AddGameObject(base);
            basesSpawned++;

            attempts++;
        }
    }

    bool Bolo::IsCellOccupied(int cellX, int cellY)
    {
        // Check if player is in this cell
        if (mPlayer)
        {
            int playerCellX = static_cast<int>(mPlayer->GetBounds().topLeft.x / mCellSize);
            int playerCellY = static_cast<int>(mPlayer->GetBounds().topLeft.y / mCellSize);
            if (playerCellX == cellX && playerCellY == cellY)
                return true;
        }

        // Check if any base is in this cell
        if (mLastContext && mLastContext->EngineContext)
        {
            for (const auto &gameObject : *mLastContext->EngineContext)
            {
                if (auto base = std::dynamic_pointer_cast<Base>(gameObject))
                {
                    int baseCellX = static_cast<int>(base->GetBounds().topLeft.x / mCellSize);
                    int baseCellY = static_cast<int>(base->GetBounds().topLeft.y / mCellSize);
                    if (baseCellX == cellX && baseCellY == cellY)
                        return true;
                }
            }
        }

        return false;
    }

    void Bolo::RenderGUI(GameContext *context)
    {
        // Draw GUI background (make it large enough to fill the GUI area)
        Rect guiBackground(mGuiXStart, 0, 500, 1200);
        context->ScreenContext->DrawRect(guiBackground, RGBColor(32, 32, 32));

        // Game title
        context->ScreenContext->DrawText(
            "BOLO",
            30,
            Point2D(mGuiXStart + 80, 20),
            RGBColor(255, 255, 0));

        // Early return if not in gameplay or end level
        if (mState != GameState::GamePlay && mState != GameState::EndLevel)
            return;

        // Render components
        RenderRadar(context);
        RenderDirectionFinder(context);
        RenderScore(context);

        // Remaining bases
        context->ScreenContext->DrawText(
            "Bases: " + std::to_string(CountRemainingBases()),
            20,
            Point2D(mGuiXStart + 20, 700),
            RGBColor(255, 255, 255));

        // Level
        context->ScreenContext->DrawText(
            "Level: " + std::to_string(mLevel),
            20,
            Point2D(mGuiXStart + 20, 730),
            RGBColor(255, 255, 255));
    }

    void Bolo::RenderRadar(GameContext *context)
    {
        if (!mPlayer)
            return;

        // Radar display at top of GUI
        int radarSize = 200;
        int radarX = mGuiXStart + 28;
        int radarY = 80;

        // Draw radar border
        Rect radarBorder(radarX - 2, radarY - 2, radarSize + 4, radarSize + 4);
        context->ScreenContext->DrawRect(radarBorder, RGBColor(100, 100, 100));

        // Draw radar background
        Rect radarBg(radarX, radarY, radarSize, radarSize);
        context->ScreenContext->DrawRect(radarBg, RGBColor(0, 0, 0));

        // Draw player position on radar
        float playerX = mPlayer->GetBounds().topLeft.x;
        float playerY = mPlayer->GetBounds().topLeft.y;

        float radarPlayerX = radarX + (playerX / mWorldSize) * radarSize;
        float radarPlayerY = radarY + (playerY / mWorldSize) * radarSize;

        context->ScreenContext->DrawCircle(
            Point2D(radarPlayerX, radarPlayerY),
            3,
            RGBColor(0, 255, 0));

        // Label
        context->ScreenContext->DrawText(
            "Radar",
            16,
            Point2D(radarX, radarY + radarSize + 10),
            RGBColor(200, 200, 200));
    }

    void Bolo::RenderDirectionFinder(GameContext *context)
    {
        if (!mPlayer)
            return;

        // Direction finder below radar
        int finderSize = 60;
        int finderX = mGuiXStart + 98;
        int finderY = 350;

        float playerX = mPlayer->GetBounds().topLeft.x;
        float playerY = mPlayer->GetBounds().topLeft.y;

        // Check each quadrant for bases
        bool hasTop = false, hasBottom = false, hasLeft = false, hasRight = false;

        if (mLastContext && mLastContext->EngineContext)
        {
            for (const auto &gameObject : *mLastContext->EngineContext)
            {
                if (auto base = std::dynamic_pointer_cast<Base>(gameObject))
                {
                    if (base->IsAlive())
                    {
                        float baseX = base->GetBounds().topLeft.x;
                        float baseY = base->GetBounds().topLeft.y;

                        if (baseY <= playerY)
                            hasTop = true;
                        if (baseY >= playerY)
                            hasBottom = true;
                        if (baseX <= playerX)
                            hasLeft = true;
                        if (baseX >= playerX)
                            hasRight = true;
                    }
                }
            }
        }

        // Draw 4 quadrant squares
        int squareSize = finderSize / 2;

        // Top-left
        RGBColor tlColor = (hasTop && hasLeft) ? RGBColor(255, 255, 255) : RGBColor(50, 50, 50);
        Rect tlRect(finderX, finderY, squareSize, squareSize);
        context->ScreenContext->DrawRect(tlRect, tlColor);

        // Top-right
        RGBColor trColor = (hasTop && hasRight) ? RGBColor(255, 255, 255) : RGBColor(50, 50, 50);
        Rect trRect(finderX + squareSize, finderY, squareSize, squareSize);
        context->ScreenContext->DrawRect(trRect, trColor);

        // Bottom-left
        RGBColor blColor = (hasBottom && hasLeft) ? RGBColor(255, 255, 255) : RGBColor(50, 50, 50);
        Rect blRect(finderX, finderY + squareSize, squareSize, squareSize);
        context->ScreenContext->DrawRect(blRect, blColor);

        // Bottom-right
        RGBColor brColor = (hasBottom && hasRight) ? RGBColor(255, 255, 255) : RGBColor(50, 50, 50);
        Rect brRect(finderX + squareSize, finderY + squareSize, squareSize, squareSize);
        context->ScreenContext->DrawRect(brRect, brColor);

        // Label
        context->ScreenContext->DrawText(
            "Direction Finder",
            16,
            Point2D(finderX - 20, finderY + finderSize + 15),
            RGBColor(200, 200, 200));
    }

    void Bolo::RenderScore(GameContext *context)
    {
        // Score display
        context->ScreenContext->DrawText(
            "Score: " + std::to_string(mScore),
            20,
            Point2D(mGuiXStart + 20, 500),
            RGBColor(255, 255, 255));
    }

    int Bolo::CountRemainingBases()
    {
        int count = 0;
        if (mLastContext && mLastContext->EngineContext)
        {
            for (const auto &gameObject : *mLastContext->EngineContext)
            {
                if (auto base = std::dynamic_pointer_cast<Base>(gameObject))
                {
                    if (base->IsAlive())
                    {
                        count++;
                    }
                }
            }
        }
        return count;
    }

    void Bolo::CleanupLevel(GameContext *context)
    {
        // Kill all level objects (walls, bases, enemies, bullets, explosions, player)
        if (!context->EngineContext)
            return;

        // kill and clear the player reference
        if (mPlayer)
        {
            mPlayer->Kill();
            mPlayer.reset();
        }

        // Then kill all other objects in the engine
        for (const auto &gameObject : *context->EngineContext)
        {
            // Kill everything except the Bolo game manager itself
            if (gameObject.get() == this)
                continue;

            // Kill walls, bases, enemies, bullets, explosions
            gameObject->Kill();
        }
    }

    bool Bolo::AllBasesDestroyed()
    {
        return CountRemainingBases() == 0;
    }

    void Bolo::ReceiveNotification(const std::string &message)
    {
        if (message == "BASE_DESTROYED")
        {
            mScore += 100;
        }
        else if (message == "ENEMY_DESTROYED")
        {
            mScore += 1;
        }
    }
}