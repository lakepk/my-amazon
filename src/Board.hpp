#ifndef BOARD_HPP
#define BOARD_HPP

#include <cmath>
#include <vector>

// 定义格子状态
enum TileState { EMPTY = 0, WHITE_QUEEN = 1, BLACK_QUEEN = 2, ARROW = 3 };

class AmazonBoard {
public:
    int grid[8][8]; // 棋盘数据

    AmazonBoard();  // 构造函数：初始化棋盘

    // 判断从 (x1, y1) 到 (x2, y2) 是否可以移动或射箭
    bool IsPathClear(int x1, int y1, int x2, int y2) const;

    // 获取某个位置的状态
    int GetPiece(int x, int y) const;
    
    // 修改某个位置的状态
    void SetPiece(int x, int y, int type);
};

#endif