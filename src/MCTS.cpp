#include "MCTS.hpp"
#include <algorithm>
#include <random>
#include <map>

AmazonMove MCTS::GetBestMove(AmazonBoard currentBoard, int aiPlayer, int iterations) {
    auto root = std::make_unique<MCTSNode>(currentBoard, aiPlayer);
    
    for (int i = 0; i < iterations; ++i) {
        MCTSNode* curr = root.get();
        
        // 1. Selection & Expansion 
        if (curr->children.empty()) {
            std::vector<AmazonMove> moves = getAllLegalMoves(curr->board, curr->playerToMove);
            for (auto& m : moves) {
                AmazonBoard nextBoard = curr->board;
                // 执行动作
                nextBoard.grid[m.qy1][m.qx1] = EMPTY;
                nextBoard.grid[m.qy2][m.qx2] = curr->playerToMove;
                nextBoard.grid[m.ay][m.ax] = ARROW;

                //这是修改后的
                auto newNode = std::make_unique<MCTSNode>(nextBoard,3-curr->playerToMove,curr);
                newNode->move = m;
                curr->children.push_back(std::move(newNode));
                //curr->children.push_back(std::make_unique<MCTSNode>(nextBoard, 3 - curr->playerToMove, curr));（这里有问题，导致了bot棋子的增加，但是没搞清楚具体为啥）
            }
        }
        
        // 2. Simulation (模拟)，现在进行了简单的评估
        if (!curr->children.empty()) {
            //这里是随机找一个走法走，现在引入了评估函数，需要修改
            MCTSNode* pick = curr->selectChild();
            double result = simulate(pick->board, pick->playerToMove);
            
            // 3. Backpropagation (回传)，让最底下这一步以及这一步的所有上级步骤visits++，这里可以不改
            MCTSNode* back = pick;
            while (back) {
                back->visits++;
                back->wins += result;
                back = back->parent;
            }
        }
    }

    // 返回访问次数最多的动作
    double maxAverageScores = -1;
    AmazonMove bestMove;
    for (auto& child : root->children) {
        double thisAverageScores = child->wins/(double)child->visits;
        if (thisAverageScores > maxAverageScores) {
            maxAverageScores = thisAverageScores;
            bestMove = child->move;
        }
    }
    return bestMove;
}

// 模拟函数：随机走子直到没地方可走(经过修改我让它只走十步。但是bot应该会变得更加智能，因为我添加了评估函数)
double MCTS::simulate(AmazonBoard tempBoard, int p) {
    // 设置最大模拟步数，防止死循环（虽然亚马逊棋必然会结束，但限制步数能提高效率）
    int maxSteps = 10; 
    int currentP = p;
    
    for (int step = 0; step < maxSteps; ++step) { 
        // 1. 获取当前玩家的所有合法动作
        // 注意：这里必须调用你已经写好的 getAllLegalMoves
        std::vector<AmazonMove> moves = getAllLegalMoves(tempBoard, currentP);

        // 2. 如果没有合法动作，说明当前玩家输了
        if (moves.empty()) {
            // 返回赢家：如果是玩家1没法走了，玩家2赢；反之亦然
            return (currentP == 1) ? 0 : 1;//如果赢了就给一个比较好的分数
        }

        // 3. 利用 rand() 随机选择一个动作
        int randomIndex = rand() % moves.size();
        AmazonMove m = moves[randomIndex];

        // 4. 在临时棋盘上执行这个动作
        tempBoard.grid[m.qy1][m.qx1] = 0;      // 原位置清空
        tempBoard.grid[m.qy2][m.qx2] = currentP; // 移动女王
        tempBoard.grid[m.ay][m.ax] = 3;        // 射出箭（假设 3 是障碍物/箭）

        // 5. 切换玩家
        currentP = (currentP == 1) ? 2 : 1;
    }
    double botMoves = (double)getAllLegalMoves(tempBoard,1).size()+1;
    double playerMoves = (double)getAllLegalMoves(tempBoard,2).size()+1;
    double score = double(botMoves/(playerMoves+botMoves));
    // 如果达到最大步数还没分胜负（极少见），可以返回平局 0 或根据领地估分
    return score; 
}

// 我需要一个评估函数，来找到对bot最有利的走法。“最有利”的衡量是合法步数之比。
double MCTS::evaluateBoard(const AmazonBoard& mBoard, int mPlayer){
    auto myMoves = getAllLegalMoves(mBoard, mPlayer).size();
    auto enemyMoves = getAllLegalMoves(mBoard, 3-mPlayer).size();
    return (double)(myMoves/enemyMoves);
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