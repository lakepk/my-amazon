#include "raylib.h"
#include "Board.hpp"
#include <ctime>
#include "MCTS.hpp"
#include <string>
#include "GameManager.hpp"
const int screenWidth = 800;
const int screenHeight = 800;
const int gridSize = 8; // 棋盘大小为8x8
const int cellSize = screenWidth / gridSize;

MCTS myCleverBot;

GameManager gm;

void InitAmazons() {
    gm.board.grid[0][2] = 2; gm.board.grid[0][5] = 2; // 黑方
    gm.board.grid[2][0] = 2; gm.board.grid[2][7] = 2; // 黑方
    gm.board.grid[7][2] = 1; gm.board.grid[7][5] = 1; // 白方
    gm.board.grid[5][0] = 1; gm.board.grid[5][7] = 1; // 白方
}

void boardClear(){
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            gm.board.grid[i][j] = EMPTY;
        }
    }
}

void endingCG(int player){
        DrawText(player == 1 ? "You win!" : "Bot win!",screenWidth/2-80,screenHeight/2-30,50,RED);
        DrawText("Press ENTER to exit",screenWidth/2-130,screenHeight/2+30,25,BLUE);
        DrawText("Press SPACE to play again",screenWidth/2-160,screenHeight/2+60,25,BLUE);
}

int turn = 1;//回合数

int currentPlayer = 2; // 1是bot 2是你
int gameState = 0; // 0: 选子, 1: 移动, 2: 射箭
Vector2 selectedIdx = {-1, -1};
bool gameover = 0;

void DrawBoardScene(bool isOver) {
    for (int y = 0; y < gridSize; y++) {
        for (int x = 0; x < gridSize; x++) {
            Color c = ((x + y) % 2 == 0) ? LIGHTGRAY : GRAY;
            DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, c);
            
            // 只有在玩家回合且没结束时显示高亮
            if (!isOver && currentPlayer == 2) {
                bool isHighlit = false;
                if (gameState == 1 || gameState == 2) {
                    if (gm.board.IsPathClear((int)selectedIdx.x, (int)selectedIdx.y, x, y)) isHighlit = true;
                }
                if (gameState > 0 && x == selectedIdx.x && y == selectedIdx.y) DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, YELLOW);
                else if (isHighlit) DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, LIME);
            }

            if (gm.board.grid[y][x] == 1) DrawCircle(x*cellSize + cellSize/2, y*cellSize + cellSize/2, cellSize*0.4, WHITE);
            if (gm.board.grid[y][x] == 2) DrawCircle(x*cellSize + cellSize/2, y*cellSize + cellSize/2, cellSize*0.4, BLACK);
            if (gm.board.grid[y][x] == 3) DrawPoly({(float)x*cellSize + cellSize/2, (float)y*cellSize + cellSize/2}, 4, cellSize*0.3, 45, RED);
        }
    }
}

double displaySaveMessageUntil = 0.0;

int main() {
    srand((unsigned int)time(NULL));
    InitWindow(screenWidth, screenHeight, "Game of the Amazons");
    InitAmazons();
    SetTargetFPS(60);
    bool needToCheckGameOver = true;

    while (!WindowShouldClose()) {
        AmazonMove humanMove;
        if(IsKeyPressed(KEY_TAB)) gm.currentScene = MENU;
        if (gm.currentScene == MENU) {
            if (IsKeyPressed(KEY_N)) gm.StartNewGame(); gm.history.clear();
            if (IsKeyPressed(KEY_L)) gm.LoadGame("save.dat");
        }else if(gm.currentScene == REPLAY){
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                gm.NextReplayStep();
            }
            if(IsKeyPressed(KEY_R)) gm.currentScene = PLAYING;
        }else if(gm.currentScene == PLAYING){
            if (IsKeyPressed(KEY_S)) {
                if(gm.SaveGame("save.dat")){
                    displaySaveMessageUntil = GetTime() + 2.0f;
                }
            }
            if(IsKeyPressed(KEY_R)){
                gm.currentScene = REPLAY;
                gm.EnterReplayMode();
            }
            

            // checkpoint；检测游戏是否已经结束
            if(!gameover&&needToCheckGameOver){
                std::vector<AmazonMove> nextMoves = myCleverBot.getAllLegalMoves(gm.board,currentPlayer);
                if(nextMoves.empty()){
                    gameover = 1;
                }
                needToCheckGameOver = false;
            }
            if(!gameover){  
                // 人机回合！
                if(currentPlayer == 1){

                    AmazonMove botMove = myCleverBot.GetBestMove(gm.board,currentPlayer);
                    gm.history.push_back(botMove);//便于复盘
                    gm.board.SetPiece(botMove.qx1,botMove.qy1,EMPTY);
                    gm.board.SetPiece(botMove.qx2,botMove.qy2,currentPlayer);
                    gm.board.SetPiece(botMove.ax,botMove.ay,ARROW);
                    currentPlayer = 2;
                    gameState = 0;
                    gm.turn++;
                    needToCheckGameOver = true;
                }
                // ore no turn!
                if (currentPlayer == 2&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

                    Vector2 mPos = GetMousePosition();
                    int x = mPos.x / cellSize;
                    int y = mPos.y / cellSize;

                    if (gameState == 0) { // 选子阶段
                        if (gm.board.grid[y][x] == currentPlayer) {
                            selectedIdx = {(float)x, (float)y};
                            humanMove.qx1 = selectedIdx.x;
                            humanMove.qy1 = selectedIdx.y;
                            gameState = 1;
                        }
                    } 
                    // 在移动阶段 (gameState == 1)
                    else if (gameState == 1) { 
                        //首先来个反悔机制
                        if (x == (int)selectedIdx.x && y == (int)selectedIdx.y){
                            gameState = 0;
                            selectedIdx={-1,-1};

                        }

                        // 使用写好的 IsPathClear 进行合法性判定
                        else if (gm.board.IsPathClear((int)selectedIdx.x, (int)selectedIdx.y, x, y)) {
                            gm.board.grid[(int)selectedIdx.y][(int)selectedIdx.x] = EMPTY;
                            gm.board.grid[y][x] = currentPlayer;
                            selectedIdx = {(float)x, (float)y}; 
                            humanMove.qx2 = selectedIdx.x;
                            humanMove.qy2 = selectedIdx.y;
                            gameState = 2; // 进入射箭阶段
                        }
                    } 
                    // 在射箭阶段 (gameState == 2)
                    else if (gameState == 2) { 
                        if (gm.board.IsPathClear((int)selectedIdx.x, (int)selectedIdx.y, x, y)) {
                            gm.board.grid[y][x] = ARROW;
                            humanMove.ax = x;
                            humanMove.ay = y;
                            gm.RecordMove(humanMove); 
                            currentPlayer = 1; 
                            gameState = 0;
                            selectedIdx = {-1.-1};
                            needToCheckGameOver = true;
                        }
                    }
                }
            }
            else {
                if(IsKeyPressed(KEY_SPACE)){
                    gameover = false;
                    boardClear();
                    InitAmazons();
                    currentPlayer = 2;
                    gameState = 0;
                    selectedIdx = {-1,-1};
                    gm.history.clear();
                    gm.turn = 1;
                    needToCheckGameOver = true;
                }
                if(IsKeyPressed(KEY_ENTER)) break;
            }
        }else if(gm.currentScene == REPLAY){
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) gm.NextReplayStep();
            if (IsKeyPressed(KEY_ESCAPE)) gm.currentScene = MENU;
        }
        //现在绘制界面

        BeginDrawing();
        gm.Draw(gameState,selectedIdx,gameover);//注意Draw函数内部是没有begin和end的
        if(GetTime()<displaySaveMessageUntil){
            DrawRectangle(250,350,300,50,Fade(DARKGRAY,0.8f));
            DrawText("SAVE SUCCESSFUL!",300,365,20,GREEN);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}