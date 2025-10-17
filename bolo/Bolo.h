#ifndef BOLO_H
#define BOLO_H

#include "CollisionObject.h"
#include "MathUtil.h"
#include "GameContext.h"
#include "Player.h"
#include "Base.h"
#include "NotificationManager.h"
#include <vector>
#include <memory>

namespace CMPUT350
{
    enum class GameState
    {
        WaitingToStart,    // Waiting for player to choose density
        InitializingLevel, // Setting up level
        GamePlay,          // Active gameplay
        EndLevel,          // Level complete
        CleaningUp         // Removing objects between levels
    };

    class Bolo : public GameObject, public std::enable_shared_from_this<Bolo>
    {
    public:
        Bolo(int mazeWidth, int mazeHeight, int cellSize, int initialLevel);

        void Update(GameContext *context) override;
        void LateUpdate(GameContext *context) override;
        void RenderBackground(GameContext *context) override;
        void RenderForeground(GameContext *context) override;
        bool HandleKeyEvent(GameContext *context, char key) override;
        void ReceiveNotification(const std::string &message) override;
        const Rect &GetBounds();

        // bool IsAlive() override;

    private:
        // Game state
        GameState mState;
        int mLevel;
        unsigned mScore = 0;
        int mMazeDensity; // 1-5
        int mStateTimer;  // For timing state transitions
        std::shared_ptr<Player> mPlayer;
        bool mIsRegistered = false; // Track if we're registered for notifications

        // Maze data - set from constructor parameters
        int mMazeWidth;
        int mMazeHeight;
        int mCellSize;
        int mWorldSize; // mMazeWidth * mCellSize

        // Maze walls: [y][x][direction] where direction: 0=right, 1=down
        std::vector<std::vector<std::vector<bool>>> mWalls;
        std::vector<std::vector<bool>> mCellExplored;

        // GUI layout
        int mGameplayWidth;
        int mGuiXStart;

        Rect mBounds;

        // Maze generation
        void GenerateMaze();
        void GenerateFullMaze();
        void RemoveWalls(int density);
        void AddWallsToEngine(GameContext *context);
        void ClearMaze();
        void DFS(int startX, int startY);

        // Level management
        void InitializeLevel(GameContext *context);
        void CleanupLevel(GameContext *context);
        void SpawnBases(GameContext *context);
        void SpawnPlayer(GameContext *context);
        bool IsCellOccupied(int cellX, int cellY);

        // GUI rendering
        void RenderGUI(GameContext *context);
        void RenderRadar(GameContext *context);
        void RenderDirectionFinder(GameContext *context);
        void RenderScore(GameContext *context);

        // Game state checks
        int CountRemainingBases();
        bool AllBasesDestroyed();
    };
}

#endif