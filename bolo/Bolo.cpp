#include "Bolo.h"
#include "Wall.h"
#include "Enemy.h"
#include "../engine/DrawContext.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <stack>

namespace CMPUT350
{
    Bolo::Bolo(int mazeWidth, int mazeHeight, int cellSize, int initialLevel)
        : mState(GameState::WaitingToStart),
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
        case GameState::WaitingToStart:
            // Waiting for player to press 1-5 to select density
            break;

        case GameState::InitializingLevel:
            // Generate maze and spawn objects
            GenerateMaze();
            SpawnPlayer(context);
            SpawnBases(context);
            AddWallsToEngine(context);

            if (context->NotificationContext && !mIsRegistered)
            {
                auto self = shared_from_this();
                context->NotificationContext->Register(self, "BASE_DESTROYED");
                context->NotificationContext->Register(self, "ENEMY_DESTROYED");
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
                mStateTimer = 150; // 5 seconds at 30 FPS
            }
            break;

        case GameState::EndLevel:
            if (mStateTimer > 0)
            {
                mStateTimer--;
            }
            else
            {
                mState = GameState::CleaningUp;
                mStateTimer = 10; // Wait 10 frames
            }
            break;

        case GameState::CleaningUp:
            if (mStateTimer > 0)
            {
                mStateTimer--;
            }
            else
            {
                // Move to next level
                mLevel++;
                mState = GameState::InitializingLevel;
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

            // Center camera on player
            if (mPlayer)
            {
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

        // Render state-specific text
        if (mState == GameState::WaitingToStart)
        {
            // Display instructions
            context->ScreenContext->DrawText(
                "Press 1-5 to select",
                20,
                Point2D(mGuiXStart + 20, 400),
                RGBColor(255, 255, 255));
            context->ScreenContext->DrawText(
                "maze density",
                20,
                Point2D(mGuiXStart + 20, 430),
                RGBColor(255, 255, 255));
        }
        else if (mState == GameState::EndLevel)
        {
            // Display congratulations screen (centered in gameplay area)
            float centerX = mGameplayWidth / 2.0f;
            float centerY = 512.0f;

            // Draw background overlay
            Rect overlay(0, 0, mGameplayWidth, 1024);
            context->ScreenContext->DrawRect(overlay, RGBColor(0, 0, 0)); // Black background

            // Main congratulations text
            std::string congratsText = (mLevel == 1) ? "FIRST LEVEL COMPLETE!" : "LEVEL COMPLETE!";
            context->ScreenContext->DrawText(
                congratsText,
                40,
                Point2D(centerX - 200, centerY - 100),
                RGBColor(0, 255, 0));

            // Level number
            context->ScreenContext->DrawText(
                "Level " + std::to_string(mLevel) + " Finished",
                24,
                Point2D(centerX - 120, centerY - 40),
                RGBColor(255, 255, 255));

            // Score display
            context->ScreenContext->DrawText(
                "Score: " + std::to_string(mScore),
                24,
                Point2D(centerX - 80, centerY + 20),
                RGBColor(255, 255, 0));

            // Countdown timer
            int secondsLeft = (mStateTimer + 29) / 30; // Convert frames to seconds
            context->ScreenContext->DrawText(
                "Next level in " + std::to_string(secondsLeft) + " seconds...",
                18,
                Point2D(centerX - 150, centerY + 80),
                RGBColor(200, 200, 200));
        }

        // Restore gameplay viewport
        if (mState == GameState::GamePlay || mState == GameState::EndLevel)
        {
            context->ScreenContext->SetGameplayViewport();

            if (mPlayer)
            {
                float playerX = mPlayer->GetBounds().topLeft.x + mPlayer->GetBounds().width / 2.0f;
                float playerY = mPlayer->GetBounds().topLeft.y + mPlayer->GetBounds().height / 2.0f;
                context->ScreenContext->SetContextCenter(Point2D(playerX, playerY));
            }
        }
    }

    bool Bolo::HandleKeyEvent(GameContext *context, char key)
    {
        if (mState == GameState::WaitingToStart)
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
        GenerateFullMaze();
        RemoveWalls(mMazeDensity);
    }

    void Bolo::GenerateFullMaze()
    {
        // Randomized DFS maze generation
        ClearMaze();

        // Start DFS from (0, 0)
        DFS(0, 0);
    }

    void Bolo::DFS(int x, int y)
    {
        // Mark current cell as explored
        mCellExplored[y][x] = true;

        // Create vector of 4 directions and shuffle them
        std::vector<std::vector<int>> directions = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}}; // Right, Down, Left, Up

        // Shuffle directions for randomization
        for (int i = 3; i > 0; i--)
        {
            int j = std::rand() % (i + 1);
            std::swap(directions[i][0], directions[j][0]);
            std::swap(directions[i][1], directions[j][1]);
        }

        // Try each direction
        for (int i = 0; i < 4; i++)
        {
            int nx = x + directions[i][0];
            int ny = y + directions[i][1];

            // Check if next cell is valid and unvisited
            if (nx >= 0 && nx < mMazeWidth && ny >= 0 && ny < mMazeHeight && !mCellExplored[ny][nx])
            {
                // Remove wall between current and next cell
                if (directions[i][0] == 1) // Moving right
                {
                    mWalls[y][x][0] = false;
                }
                else if (directions[i][0] == -1) // Moving left
                {
                    mWalls[ny][nx][0] = false;
                }
                else if (directions[i][1] == 1) // Moving down
                {
                    mWalls[y][x][1] = false;
                }
                else if (directions[i][1] == -1) // Moving up
                {
                    mWalls[ny][nx][1] = false;
                }

                // Recursively visit next cell
                DFS(nx, ny);
            }
        }
    }

    void Bolo::RemoveWalls(int density)
    {
        // Calculate target percentage of walls to keep
        // density 1: (5 + 2*1)/18 = 7/18 = 38.8%
        // density 5: (5 + 2*5)/18 = 15/18 = 83.3%
        float keepPercentage = (5.0f + 2.0f * density) / 18.0f;

        // Count total interior walls (excluding outer walls)
        int totalWalls = 0;
        for (int y = 0; y < mMazeHeight; y++)
        {
            for (int x = 0; x < mMazeWidth; x++)
            {
                if (x < mMazeWidth - 1 && mWalls[y][x][0])
                    totalWalls++; // Right wall
                if (y < mMazeHeight - 1 && mWalls[y][x][1])
                    totalWalls++; // Down wall
            }
        }

        int targetWalls = static_cast<int>(totalWalls * keepPercentage);
        int wallsToRemove = totalWalls - targetWalls;

        // Randomly remove walls (but not outer walls)
        while (wallsToRemove > 0)
        {
            int x = std::rand() % mMazeWidth;
            int y = std::rand() % mMazeHeight;
            int dir = std::rand() % 2;

            // Check if this is an outer wall
            bool isOuterWall = false;
            if (dir == 0 && x == mMazeWidth - 1)
                isOuterWall = true; // Right edge

            if (dir == 1 && y == mMazeHeight - 1)
                isOuterWall = true; // Bottom edge

            if (dir == 0 && x == 0 && !mWalls[y][x][0])
                continue; // Left edge (wall to the left)

            if (dir == 1 && y == 0 && !mWalls[y][x][1])
                continue; // Top edge (wall above)

            if (!isOuterWall && mWalls[y][x][dir])
            {
                mWalls[y][x][dir] = false;
                wallsToRemove--;
            }
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

            auto base = std::make_shared<Base>(Point2D(worldX, worldY));
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
        if (mState != GameState::GamePlay && mState != GameState::EndLevel)
            return;

        // Draw GUI background (make it large enough to fill the GUI area)
        Rect guiBackground(mGuiXStart, 0, 500, 1200);
        context->ScreenContext->DrawRect(guiBackground, RGBColor(32, 32, 32));

        // Game title
        context->ScreenContext->DrawText(
            "BOLO",
            30,
            Point2D(mGuiXStart + 80, 20),
            RGBColor(255, 255, 0));

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