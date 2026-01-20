#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP

#include "raylib.h"
#include "Board.hpp"
#include "MCTS.hpp"
#include <vector>
#include <string>

enum GameScene { MENU, PLAYING, REPLAY };

class GameManager {
public:
    // --- 状态变量 ---
    GameScene currentScene = MENU;
    int currentPlayer = 2; // 1: Bot, 2: Player
    int turn = 1;
    int replayIndex = 0;   // 复盘进度指针

    // --- 数据对象 ---
    AmazonBoard board;
    std::vector<AmazonMove> history; // 动作历史记录

    // --- 核心逻辑函数 ---
    void StartNewGame();
    bool SaveGame(const std::string& filename);
    bool LoadGame(const std::string& filename);
    void RecordMove(AmazonMove m);

    // --- 复盘功能函数 ---
    void EnterReplayMode();
    void NextReplayStep();

    // --- UI 与 绘图函数 ---
    // 传入 gameState(阶段), selectedIdx(选子坐标), isGameOver(结束标志) 供界面渲染
    void Draw(int gameState, Vector2 selectedIdx, bool isGameOver);
};
#endif