#include "MCTS.hpp"
#include <algorithm>
#include <random>
#include <map>

AmazonMove MCTS::GetBestMove(AmazonBoard currentBoard, int aiPlayer, int iterations) {
    // 先检查根节点是否有合法走法，没有就直接返回一个空动作
    std::vector<AmazonMove> rootMoves = getAllLegalMoves(currentBoard, aiPlayer);
    if (rootMoves.empty()) {
        return AmazonMove{0, 0, 0, 0, 0, 0};
    }

    auto root = std::make_unique<MCTSNode>(currentBoard, aiPlayer);

    // 使用一个固定的随机数引擎，比 rand() 更稳定
    static thread_local std::mt19937 rng(std::random_device{}());

    for (int i = 0; i < iterations; ++i) {
        MCTSNode* node = root.get();

        // 1. Selection：沿着 UCB1 一直往下走，直到叶子或终局
        while (!node->children.empty()) {
            // 如果这个节点已经无子可走，相当于终局，直接停止选择
            if (getAllLegalMoves(node->board, node->playerToMove).empty()) {
                break;
            }
            node = node->selectChild();
        }

        // 2. Expansion：如果不是终局，则展开一次（生成子节点）
        std::vector<AmazonMove> moves = getAllLegalMoves(node->board, node->playerToMove);
        if (!moves.empty()) {
            if (node->children.empty()) {
                node->children.reserve(moves.size());
                for (auto& m : moves) {
                    AmazonBoard nextBoard = node->board;
                    // 执行动作
                    nextBoard.grid[m.qy1][m.qx1] = EMPTY;
                    nextBoard.grid[m.qy2][m.qx2] = node->playerToMove;
                    nextBoard.grid[m.ay][m.ax] = ARROW;

                    auto newNode = std::make_unique<MCTSNode>(nextBoard, 3 - node->playerToMove, node);
                    newNode->move = m;
                    node->children.push_back(std::move(newNode));
                }
            }

            // 从当前节点的孩子中选择一个用于模拟：优先选未访问的，否则再用 UCB
            MCTSNode* next = nullptr;
            for (auto& ch : node->children) {
                if (ch->visits == 0) {
                    next = ch.get();
                    break;
                }
            }
            if (!next && !node->children.empty()) {
                next = node->selectChild();
            }
            if (next) {
                node = next;
            }
        }

        // 3. Simulation：从选中的节点开始随机模拟，对 AI 视角打分
        double result = simulate(node->board, node->playerToMove, aiPlayer);

        // 4. Backpropagation：沿父链回溯，将结果累加到路径上的所有节点
        MCTSNode* back = node;
        while (back) {
            back->visits++;
            back->wins += result; // result 始终是从 aiPlayer 视角的“好坏”
            back = back->parent;
        }
    }

    // 选平均得分最高的根节点子节点作为最终落子
    AmazonMove bestMove = rootMoves[0];
    double bestScore = -1.0;
    for (auto& child : root->children) {
        if (child->visits == 0) continue;
        double avg = child->wins / static_cast<double>(child->visits);
        if (avg > bestScore) {
            bestScore = avg;
            bestMove = child->move;
        }
    }
    return bestMove;
}

// 模拟函数：从某个节点开始随机走，结果始终从 aiPlayer 视角来评估
double MCTS::simulate(AmazonBoard tempBoard, int currentPlayer, int aiPlayer) {
    // 设置最大模拟步数，防止死循环
    const int maxSteps = 20;

    static thread_local std::mt19937 rng(std::random_device{}());

    for (int step = 0; step < maxSteps; ++step) {
        std::vector<AmazonMove> moves = getAllLegalMoves(tempBoard, currentPlayer);

        // 没有合法走法：当前玩家输
        if (moves.empty()) {
            // 如果当前没法走的是 AI，自然是 0 分；反之是 1 分
            return (currentPlayer == aiPlayer) ? 0.0 : 1.0;
        }

        std::uniform_int_distribution<int> dist(0, static_cast<int>(moves.size()) - 1);
        AmazonMove m = moves[dist(rng)];

        // 执行动作
        tempBoard.grid[m.qy1][m.qx1] = EMPTY;
        tempBoard.grid[m.qy2][m.qx2] = currentPlayer;
        tempBoard.grid[m.ay][m.ax] = ARROW;

        // 轮到另外一方
        currentPlayer = 3 - currentPlayer;
    }

    // 没有走到终局，用行动力（合法步数）来估分
    int myMoves = static_cast<int>(getAllLegalMoves(tempBoard, aiPlayer).size());
    int oppMoves = static_cast<int>(getAllLegalMoves(tempBoard, 3 - aiPlayer).size());
    int total = myMoves + oppMoves;
    if (total == 0) {
        return 0.5; // 双方都动不了，当成平局
    }
    return static_cast<double>(myMoves) / static_cast<double>(total);
}

// 我需要一个评估函数，来找到对bot最有利的走法。“最有利”的衡量是合法步数之比。
double MCTS::evaluateBoard(const AmazonBoard& mBoard, int mPlayer){
    auto myMoves = static_cast<int>(getAllLegalMoves(mBoard, mPlayer).size());
    auto enemyMoves = static_cast<int>(getAllLegalMoves(mBoard, 3 - mPlayer).size());

    int total = myMoves + enemyMoves;
    if (total == 0) {
        return 0.5; // 双方都被封死
    }
    return static_cast<double>(myMoves) / static_cast<double>(total);
}

// 获得一个行动方式的数组，包含该状态下所有合法的行动方式
std::vector<AmazonMove> MCTS::getAllLegalMoves(const AmazonBoard& mBoard, int player){
    std::vector<AmazonMove> allLegalMoves;
    int dx[8]={0,0,1,1,1,-1,-1,-1};
    int dy[8]={1,-1,1,-1,0,0,1,-1};
    //方向数组
    
    //找到自己的棋子
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(mBoard.grid[j][i] != player){
                continue;
            }
            else{
                // move queen
                for(int direction=0;direction<8;direction++){
                    for(int distance=1;distance<8;distance++){
                        int target_x=i+distance*dx[direction];
                        int target_y=j+distance*dy[direction];
                        if(target_x<0||target_x>7||target_y<0||target_y>7){
                            break;
                        }
                        if(mBoard.IsPathClear(i,j,target_x,target_y)){
                            // 更新棋盘，shoot arrow
                            AmazonBoard afterMove_board=mBoard;
                            afterMove_board.SetPiece(i,j,0);
                            afterMove_board.SetPiece(target_x,target_y,player);

                            afterMove_board.SetPiece(target_x,target_y,EMPTY);//临时把女王看成空，方便射箭

                            for(int direction_arrow=0;direction_arrow<8;direction_arrow++){
                                for(int distance_arrow=1;distance_arrow<8;distance_arrow++){
                                    int target_arrow_x=target_x+distance_arrow*dx[direction_arrow];
                                    int target_arrow_y=target_y+distance_arrow*dy[direction_arrow];
                                    if(target_arrow_x<0||target_arrow_x>7||target_arrow_y<0||target_arrow_y>7){
                                        break;
                                    }
                                    if(afterMove_board.IsPathClear(target_x,target_y,target_arrow_x,target_arrow_y)){
                                        allLegalMoves.push_back({i,j,target_x,target_y,target_arrow_x,target_arrow_y});
                                    }
                                }
                            }

                            afterMove_board.SetPiece(target_x,target_y,player);

                        }
                    }
                }
            }
        }
    }
    return allLegalMoves;
}