#include "GameManager.hpp"
#include <string>

// 基础常量定义
const int screenWidth = 800;
const int cellSize = 800 / 8;

void GameManager::StartNewGame() {
    board = AmazonBoard(); // 调用构造函数重置棋盘
    history.clear();
    currentPlayer = 2; // 玩家先手
    turn = 1;
    replayIndex = 0;
    currentScene = PLAYING;
}//好的这部分是对的

void GameManager::RecordMove(AmazonMove m) {
    history.push_back(m);
}//对的

bool GameManager::SaveGame(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) return false;

    // 1. 保存基础状态
    fwrite(&currentPlayer, sizeof(int), 1, file);
    fwrite(&turn, sizeof(int), 1, file);
    fwrite(board.grid, sizeof(int), 64, file);

    // 2. 保存历史记录长度和内容
    int historySize = (int)history.size();
    fwrite(&historySize, sizeof(int), 1, file);
    if (historySize > 0) {
        fwrite(history.data(), sizeof(AmazonMove), historySize, file);
    }

    fclose(file);
    return true;
}

bool GameManager::LoadGame(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) return false;

    fread(&currentPlayer, sizeof(int), 1, file);
    fread(&turn, sizeof(int), 1, file);
    fread(board.grid, sizeof(int), 64, file);

    int historySize;
    if (fread(&historySize, sizeof(int), 1, file) == 1) {
        history.resize(historySize);
        if (historySize > 0) {
            fread(history.data(), sizeof(AmazonMove), historySize, file);
        }
    }

    fclose(file);
    currentScene = PLAYING;
    return true;
}


void GameManager::EnterReplayMode() {
    board = AmazonBoard(); 
    replayIndex = 0;
    currentScene = REPLAY;
}//有点问题！！！！（不对没问题）

void GameManager::NextReplayStep() {
    if (replayIndex < (int)history.size()) {
        AmazonMove m = history[replayIndex];
        int p = (replayIndex % 2 == 0) ? 2 : 1; 
        board.SetPiece(m.qx1, m.qy1, EMPTY);
        board.SetPiece(m.qx2, m.qy2, p);
        board.SetPiece(m.ax, m.ay, ARROW);
        replayIndex++;
    }
}//对的

// --- 新增：绘制主入口 ---
void GameManager::Draw(int gameState, Vector2 selectedIdx, bool isGameOver) {
    ClearBackground(RAYWHITE);

    if (currentScene == MENU) {
        // 绘制主菜单
        DrawText("AMAZONS AI", 220, 200, 60, DARKGRAY);
        DrawRectangleLines(250, 350, 300, 50, GRAY);
        DrawText("Press [N] New Game", 285, 365, 20, BLACK);

        if (FileExists("save.dat")) {
            DrawRectangleLines(250, 420, 300, 50, DARKGREEN);
            DrawText("Press [L] Load Game", 285, 435, 20, DARKGREEN);
        }
    } 
    else {
        // 绘制棋盘背景和格位
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Color tileColor = ((x + y) % 2 == 0) ? Color{240, 217, 181, 255} : Color{181, 136, 99, 255};
                DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, tileColor);

                // 绘制玩家操作高亮
                if (currentScene == PLAYING && currentPlayer == 2 && !isGameOver) {
                    if (x == (int)selectedIdx.x && y == (int)selectedIdx.y) {
                        DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, Fade(YELLOW, 0.5f));
                    }
                    if (gameState > 0 && board.IsPathClear((int)selectedIdx.x, (int)selectedIdx.y, x, y)) {
                        DrawCircle(x * cellSize + cellSize/2, y * cellSize + cellSize/2, 5, Fade(BLACK, 0.3f));
                    }
                }

                // 绘制棋子
                int piece = board.grid[y][x];
                if (piece == WHITE_QUEEN) DrawCircle(x*cellSize + cellSize/2, y*cellSize + cellSize/2, cellSize*0.4, WHITE);
                else if (piece == BLACK_QUEEN) DrawCircle(x*cellSize + cellSize/2, y*cellSize + cellSize/2, cellSize*0.4, BLACK);
                else if (piece == ARROW) DrawPoly({(float)x*cellSize + cellSize/2, (float)y*cellSize + cellSize/2}, 4, cellSize*0.3, 45, RED);
            }
        }

        // 绘制 UI 信息栏
        DrawRectangle(0, 0, 800, 40, Fade(BLACK, 0.6f));
        std::string statusText = (currentScene == REPLAY) ? "REPLAY MODE" : (currentPlayer == 2 ? "YOUR TURN" : "BOT THINKING...");
        DrawText(statusText.c_str(), 20, 10, 20, GOLD);
        DrawText(TextFormat("TURN: %d", currentScene == PLAYING ? turn : (replayIndex == history.size() ? -1 : (replayIndex+1)/2)), 680, 10, 20, WHITE);

        if (currentScene == PLAYING) {
            DrawText("[S] SAVE | [R] REPLAY | [TAB] MENU", 250, 10, 18, LIGHTGRAY);
        } else {
            DrawText("CLICK MOUSE FOR NEXT STEP | [TAB] MENU", 220, 10, 18, LIGHTGRAY);
        }
        // 游戏结束层
        if (isGameOver) {
            DrawRectangle(0, 0, 800, 800, Fade(BLACK, 0.5f));
            DrawText(currentPlayer == 1 ? "YOU WIN!" : "BOT WIN!", 280, 350, 50, RED);
            DrawText("Press [SPACE] to Restart", 260, 450, 25, RAYWHITE);
        }
    }
    return ;
}