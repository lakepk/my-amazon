#include "Board.hpp"
#include <cmath>


AmazonBoard::AmazonBoard() {
    // 1. 先清空棋盘
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            grid[y][x] = EMPTY;
        }
    }

    // 2. 设置亚马逊棋 8x8 初始位置 (经典布局)
    // 黑方 (2)
    grid[0][2] = BLACK_QUEEN; grid[0][5] = BLACK_QUEEN;
    grid[2][0] = BLACK_QUEEN; grid[2][7] = BLACK_QUEEN;
    // 白方 (1)
    grid[5][0] = WHITE_QUEEN; grid[5][7] = WHITE_QUEEN;
    grid[7][2] = WHITE_QUEEN; grid[7][5] = WHITE_QUEEN;
}

bool AmazonBoard::IsPathClear(int x1, int y1, int x2, int y2) const {
    if (x2 < 0 || x2 >= 8 || y2 < 0 || y2 >= 8) return false;
    if (grid[y2][x2] != EMPTY) return false;

    int dx = (x2 > x1) ? 1 : (x2 < x1 ? -1 : 0);
    int dy = (y2 > y1) ? 1 : (y2 < y1 ? -1 : 0);

    if (dx == 0 && dy == 0) return false;
    if (dx != 0 && dy != 0 && std::abs(x1 - x2) != std::abs(y1 - y2)) return false;

    int curX = x1 + dx;
    int curY = y1 + dy;
    while (curX != x2 || curY != y2) {
        if (grid[curY][curX] != EMPTY) return false;
        curX += dx;
        curY += dy;
    }
    return true;
}

int AmazonBoard::GetPiece(int x, int y) const {
    return grid[y][x];
}

void AmazonBoard::SetPiece(int x, int y, int type) {
    grid[y][x] = type;
}