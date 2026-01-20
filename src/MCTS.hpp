#ifndef MCTS_HPP
#define MCTS_HPP

#include "Board.hpp"
#include <vector>
#include <memory>
#include <cmath>

// 定义一个完整的亚马逊棋动作
struct AmazonMove {
    int qx1, qy1, qx2, qy2; // 移动女王
    int ax, ay;             // 射箭
};

class MCTSNode {
public:
    AmazonBoard board;
    AmazonMove move; // 到达此状态的动作
    MCTSNode* parent;
    std::vector<std::unique_ptr<MCTSNode>> children;
    
    int visits = 0;
    double wins = 0.0f;
    int playerToMove; // 谁在该节点下棋

    MCTSNode(AmazonBoard b, int p, MCTSNode* prnt = nullptr) 
        : board(b), playerToMove(p), parent(prnt) {}

    // UCB1 公式选择最佳子节点
    MCTSNode* selectChild() {
        MCTSNode* best = nullptr;
        float bestUCB = -1e9;
        for (auto& child : children) {
            float ucb = (child->wins / (child->visits + 1e-6f)) + 
                        2.0f * std::sqrt(std::log((float)visits + 1.0f) / (child->visits + 1e-6f));
            if (ucb > bestUCB) {
                bestUCB = ucb;
                best = child.get();
            }
        }
        return best;
    }
};

class MCTS {
public:
    // 找出当前局面所有合法动作（这是最难的部分）
    std::vector<AmazonMove> getAllLegalMoves(const AmazonBoard& mBoard, int player);
    // AI 思考的主函数，返回最佳动作
    AmazonMove GetBestMove(AmazonBoard currentBoard, int aiPlayer, int iterations = 5000);
    double evaluateBoard(const AmazonBoard& mBoard, int mPlayer);
    
private:
    // 模拟随机下棋直到结束
    double simulate(AmazonBoard tempBoard, int player);
};

#endif