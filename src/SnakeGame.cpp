#include "SnakeGame.h"
#include "ScoreBoard.h"
#include "Board.h"
#include "Snake.h"
#include <time.h>
#include <random>
#include <unistd.h>
#include <vector>

#include <iostream>
SnakeGame::SnakeGame(int height, int width)
{
    setInnerWall(); //여기 추가했음
    snake = new Snake(3, height / 2, width / 4 - 3);
    snakeOnBoard(snake);
    board = Board(height, width);
    score = new Score(snake);
    score_board = ScoreBoard(height, width, score);
    score_board.init();
    score_board.addAtState();
    score_board.refresh();
    mission = new Mission(score);
    mission_board = MissionBoard(height, width, mission);
    mission_board.init();
    mission_board.refresh();
    rank = new Rank();
    rank->init();
    rank_board = RankBoard(height, width, rank);
    rank_board.init();
    rank_board.addAtState(score);
    rank_board.refresh();
    direction = LEFT;
    board.init();
    readBoard();
    game_over = false;
    gItemNum = 0;
    pItemNum = 0;
    eatTimeItem = false;
    tItemY = -1;
    tItemX = -1;
    delayTime = 5;
    eatStartStatus = false;
    setHalfDelay(delayTime);
    
    //Debug

    //포지션 값 넣어주는 거 추가했음
    for (int y = 0; y < 21; y++)
    {
        for (int x = 0; x < 21; x++)
        {
            if (gameBoard[y][x] == '1')
            {
                wallYPosition.push_back(y);
                wallXPosition.push_back(x);
            }
        }
    }

}

SnakeGame::~SnakeGame()
{
    delete score;
    delete mission;
    delete snake;
    delete rank;
}

void SnakeGame::processInput()
{
    int input = board.getInput();
    switch (input)
    {
    case KEY_LEFT:
        if (direction == RIGHT){
            game_over = true;    
        }
        else if (direction != LEFT)
        {
            setDirection(LEFT);
        }
        break;
    case KEY_RIGHT:
        if (direction == LEFT){
            game_over = true;    
        }
        else if (direction != RIGHT)
        {
            setDirection(RIGHT);
        }
        break;
    case KEY_DOWN:
        if (direction == UP){
            game_over = true;    
        }
        else if (direction != DOWN)
        {
            setDirection(DOWN);
        }
        break;
    case KEY_UP:
        if (direction == DOWN){
            game_over = true;    
        }
        else if (direction != UP)
        {
            setDirection(UP);
        }
        break;
    default:
        break;
    }
    goDirection();
    board.clear();
    readBoard();
}

void SnakeGame::updateState()
{
    mission_board.addAtState();
    mission_board.refresh();
    score_board.addAtState();
    score_board.refresh();
    rank_board.addAtState(score);
    rank_board.refresh();
}

void SnakeGame::redraw()
{
    board.refresh();
}

bool SnakeGame::checkWB(int y, int x)
{
    char referPoint = gameBoard[y][x];
    if ((referPoint == '1') || (referPoint == '4'))
    {
        game_over = true;
        return true;
    }
    else if (referPoint == 'G')
    {
        gItemNum--;
        snake->plusBody();
        return false;
    }
    else if (referPoint == 'P')
    {
        pItemNum--;
        int minusY = (*snake)[snake->getLength() - 1].getYposition();
        int minusX = (*snake)[snake->getLength() - 1].getXposition();
        snake->minusBody();
        gameBoard[minusY][minusX] = '0';
        return false;
    }
    else if(referPoint == 'T')
    {
        setEatTimeItem(true);
        setEatStartStatus(true);
        minusDelayTime();
        setHalfDelay(getDelayTime());
        return false;
    }
    else
    {
        return false;
    }
}

void SnakeGame::checkLength()
{
    int l = snake->getLength();
    if (l == 2)
    {
        game_over = true;
    }
}

bool SnakeGame::checkGate(int y, int x)
{
    char referPoint = gameBoard[y][x];
    if (referPoint == '9')
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SnakeGame::isGate(){

    //body의 포지션과 gate의 포지션으로 비교하려고 했으나, body의 좌표는 한번 생성 후에 움직일때마다 초기화 되지 않음,-> 이게 정확한 방법인데 효율성이 떨어짐.
//    for(int i=0; i<snake->getLength()-1; i++){
//        if(gate1Y == (*snake)[i].getYposition() && gate1X == (*snake)[i].getXposition()){
//            return true;
//        }else if(gate2Y == (*snake)[i].getYposition() && gate2X == (*snake)[i].getXposition()){
//            return true;
//        }
//    }

    //범위 오류가 나야하는데 왜 안나지..? 한가지 버그가 있는데 들어가는게 아니라 거기 주변을 지나가고 있어도 생성되지 않을거임.
    for(int i=0; i<4; i++){

        if( (gameBoard[gate1Y+offsetY[i]][gate1X+offsetX[i]] == '3' ||gameBoard[gate1Y+offsetY[i]][gate1X+offsetX[i]] == '4') ){
            return true;
        }
        if(  (gameBoard[gate2Y+offsetY[i]][gate1X+offsetX[i]] == '3' ||gameBoard[gate2Y+offsetY[i]][gate2X+offsetX[i]] == '4') ){
            return true;
        }
    }

    //(gate1Y+offsetY[i] >= 0 && gate1X+offsetX[i] >= 20) &&
    //(gate2Y+offsetY[i] >= 0 && gate2X+offsetX[i] >= 20) &&


    return false;

}


int SnakeGame::gateHeadY(int referY, int referX)
{
    int moveY, moveX;
    int outGateY, outGateX;
    GATETYPE t;
    if ((referY == gate1Y) && (referX == gate1X))
    {
        outGateY = gate2Y;
        outGateX = gate2X;
        if (((outGateY == 0) || (outGateY == 20)) || ((outGateX == 0) || (outGateX == 20)))
        {
            t = OUTER;
        }
        else
        {
            t = INNER;
        }
    }
    else if ((referY == gate2Y) && (referX == gate2X))
    {
        outGateY = gate1Y;
        outGateX = gate1X;
        if (((outGateY == 0) || (outGateY == 20)) || ((outGateX == 0) || (outGateX == 20)))
        {
            t = OUTER;
        }
        else
        {
            t = INNER;
        }
    }
    if (t == OUTER)
    {
        if (outGateY == 0)
        {
            moveY = outGateY + 1;
            moveX = outGateX;
            setDirection(DOWN);
        }
        else if (outGateY == 20)
        {
            moveY = outGateY - 1;
            moveX = outGateX;
            setDirection(UP);
        }
        else if (outGateX == 0)
        {
            moveY = outGateY;
            moveX = outGateX + 1;
            setDirection(RIGHT);
        }
        else if (outGateX == 20)
        {
            moveY = outGateY;
            moveX = outGateX - 1;
            setDirection(LEFT);
        }
    }
    else
    {
        char rightPoint = gameBoard[outGateY][outGateX + 1];
        char leftPoint = gameBoard[outGateY][outGateX - 1];
        char upPoint = gameBoard[outGateY - 1][outGateX];
        char downPoint = gameBoard[outGateY + 1][outGateX];

        if ((rightPoint != '1') && (leftPoint != '1') && (upPoint != '1') && (downPoint != '1'))
        {
            //inner gate의 진출방향이 자유로울 때,
            switch (direction)
            {
            case LEFT:
                moveY = outGateY;
                moveX = outGateX - 1;
                break;
            case RIGHT:
                moveY = outGateY;
                moveX = outGateX + 1;
                break;
            case UP:
                moveY = outGateY - 1;
                moveX = outGateX;
                break;
            case DOWN:
                moveY = outGateY + 1;
                moveX = outGateX;
                break;
            default:
                break;
            }
        }
        else if ((rightPoint != '1') && (leftPoint != '1') && (upPoint == '1') && (downPoint == '1'))
        {
            //inner gate의 진출방향이 좌우인경우
            switch (direction)
            {
            case LEFT:
                moveY = outGateY;
                moveX = outGateX - 1;
                break;
            case RIGHT:
                moveY = outGateY;
                moveX = outGateX + 1;
                break;
            case UP:
                moveY = outGateY;
                moveX = outGateX + 1;
                setDirection(RIGHT);
                break;
            case DOWN:
                moveY = outGateY;
                moveX = outGateX - 1;
                setDirection(LEFT);
                break;
            default:
                break;
            }
        }
        else if ((rightPoint == '1') && (leftPoint == '1') && (upPoint != '1') && (downPoint != '1'))
        {
            //inner gate의 진출방향이 상하인 경우
            switch (direction)
            {
            case LEFT:
                moveY = outGateY - 1;
                moveX = outGateX;
                setDirection(UP);
                break;
            case RIGHT:
                moveY = outGateY + 1;
                moveX = outGateX;
                setDirection(DOWN);
                break;
            case UP:
                moveY = outGateY - 1;
                moveX = outGateX;
                break;
            case DOWN:
                moveY = outGateY + 1;
                moveX = outGateX;
                break;
            default:
                break;
            }
        }
    }
    return moveY;
}

int SnakeGame::gateHeadX(int referY, int referX)
{
    int moveY, moveX;
    int outGateY, outGateX;
    GATETYPE t;
    if ((referY == gate1Y) && (referX == gate1X))
    {
        outGateY = gate2Y;
        outGateX = gate2X;
        if (((outGateY == 0) || (outGateY == 20)) || ((outGateX == 0) || (outGateX == 20)))
        {
            t = OUTER;
        }
        else
        {
            t = INNER;
        }
    }
    else if ((referY == gate2Y) && (referX == gate2X))
    {
        outGateY = gate1Y;
        outGateX = gate1X;
        if (((outGateY == 0) || (outGateY == 20)) || ((outGateX == 0) || (outGateX == 20)))
        {
            t = OUTER;
        }
        else
        {
            t = INNER;
        }
    }
    if (t == OUTER)
    {
        if (outGateY == 0)
        {
            moveY = outGateY + 1;
            moveX = outGateX;
            setDirection(DOWN);
        }
        else if (outGateY == 20)
        {
            moveY = outGateY - 1;
            moveX = outGateX;
            setDirection(UP);
        }
        else if (outGateX == 0)
        {
            moveY = outGateY;
            moveX = outGateX + 1;
            setDirection(RIGHT);
        }
        else if (outGateX == 20)
        {
            moveY = outGateY;
            moveX = outGateX - 1;
            setDirection(LEFT);
        }
    }
    else
    {
        char rightPoint = gameBoard[outGateY][outGateX + 1];
        char leftPoint = gameBoard[outGateY][outGateX - 1];
        char upPoint = gameBoard[outGateY - 1][outGateX];
        char downPoint = gameBoard[outGateY + 1][outGateX];

        if ((rightPoint != '1') && (leftPoint != '1') && (upPoint != '1') && (downPoint != '1'))
        {
            //inner gate의 진출방향이 자유로울 때,
            switch (direction)
            {
            case LEFT:
                moveY = outGateY;
                moveX = outGateX - 1;
                break;
            case RIGHT:
                moveY = outGateY;
                moveX = outGateX + 1;
                break;
            case UP:
                moveY = outGateY - 1;
                moveX = outGateX;
                break;
            case DOWN:
                moveY = outGateY + 1;
                moveX = outGateX;
                break;
            default:
                break;
            }
        }
        else if ((rightPoint != '1') && (leftPoint != '1') && (upPoint == '1') && (downPoint == '1'))
        {
            //inner gate의 진출방향이 좌우인경우
            switch (direction)
            {
            case LEFT:
                moveY = outGateY;
                moveX = outGateX - 1;
                break;
            case RIGHT:
                moveY = outGateY;
                moveX = outGateX + 1;
                break;
            case UP:
                moveY = outGateY;
                moveX = outGateX + 1;
                setDirection(RIGHT);
                break;
            case DOWN:
                moveY = outGateY;
                moveX = outGateX - 1;
                setDirection(LEFT);
                break;
            default:
                break;
            }
        }
        else if ((rightPoint == '1') && (leftPoint == '1') && (upPoint != '1') && (downPoint != '1'))
        {
            //inner gate의 진출방향이 상하인 경우
            switch (direction)
            {
            case LEFT:
                moveY = outGateY - 1;
                moveX = outGateX;
                setDirection(UP);
                break;
            case RIGHT:
                moveY = outGateY + 1;
                moveX = outGateX;
                setDirection(DOWN);
                break;
            case UP:
                moveY = outGateY - 1;
                moveX = outGateX;
                break;
            case DOWN:
                moveY = outGateY + 1;
                moveX = outGateX;
                break;
            default:
                break;
            }
        }
    }
    return moveX;
}

void SnakeGame::goDirection()
{
    int preY = (*snake)[0].getYposition();
    int preX = (*snake)[0].getXposition();
    int moveY, moveX;
    switch (direction)
    {
    case LEFT:
        moveY = preY;
        moveX = preX - 1;
        break;
    case RIGHT:
        moveY = preY;
        moveX = preX + 1;
        break;
    case UP:
        moveY = preY - 1;
        moveX = preX;
        break;
    case DOWN:
        moveY = preY + 1;
        moveX = preX;
        break;
    default:
        break;
    }

    if (checkWB(moveY, moveX))
    {
        return;
    }
    checkLength();
    // 여기서
    if (checkGate(moveY, moveX))
    {
        score->gateUse++;
        moveY = gateHeadY(moveY, moveX);
        moveX = gateHeadX(moveY, moveX);
        (*snake)[0].setYposition(moveY);
        (*snake)[0].setXposition(moveX);
    }
    else
    {
        (*snake)[0].setYposition(moveY);
        (*snake)[0].setXposition(moveX);
    }
    //여기까지 수정했음
    for (int i = 1; i < snake->getLength(); i++)
    {
        int tmpY = (*snake)[i].getYposition();
        int tmpX = (*snake)[i].getXposition();
        (*snake)[i].setYposition(preY);
        (*snake)[i].setXposition(preX);
        preY = tmpY;
        preX = tmpX;
    }
    gameBoard[preY][preX] = '0';
    snakeOnBoard(snake);
}

void SnakeGame::readBoard()
{
    for (int i = 0; i < 21; i++)
    { //21로 수정
        for (int j = 0; j < 21; j++)
        { //21로 수정
            char point = gameBoard[i][j];
            if (point == '3')
            {
                board.addAt(i, j * 2, 'O');
            }
            else if (point == '4')
            {
                board.addAt(i, j * 2, ACS_BOARD);
            }
            else if (point == 'G')
            {
                board.addAt(i, j * 2, '+');
            }
            else if (point == 'P')
            {
                board.addAt(i, j * 2, '-');
            }
            else if (point == '1')
            { //임시로 innerWall 출력하게 만들었음
                if (((i != 0) && (i != 20)) && ((j != 0) && (j != 20)))
                {
                    board.addAt(i, j * 2, 'M');
                }
            }
            else if (point == '9')
            { //임시로 게이트 출력하게 만듬
                board.addAt(i, j * 2, '9');
            }
            else if (point == 'T')
            {
                board.addAt(i, j * 2, 'T');
            }
        }
    }
}

void SnakeGame::makeGItem()
{
    if (gItemNum < 3)
    {
        int y = rand() % 19 + 1;
        int x = rand() % 19 + 1;
        while (true)
        {
            char refer = gameBoard[y][x];
            if (refer == '0')
            {
                gameBoard[y][x] = 'G';
                ++gItemNum;
                break;
            }
            else
            {
                y = rand() % 19 + 1;
                x = rand() % 19 + 1;
            }
        }
    }
}

void SnakeGame::makePItem()
{
    if (pItemNum < 3)
    {
        int y = rand() % 19 + 1;
        int x = rand() % 19 + 1;
        while (true)
        {
            char refer = gameBoard[y][x];
            if (refer == '0')
            {
                gameBoard[y][x] = 'P';
                ++pItemNum;
                break;
            }
            else
            {
                y = rand() % 19 + 1;
                x = rand() % 19 + 1;
            }
        }
    }
}

void SnakeGame::snakeOnBoard(Snake *s)
{
    for (int i = 0; i < s->getLength(); i++)
    {
        int y = (*s)[i].getYposition();
        int x = (*s)[i].getXposition();
        PART p = (*s)[i].getPart();
        if (p == HEAD)
        {
            gameBoard[y][x] = '3';
        }
        else
        {
            gameBoard[y][x] = '4';
        }
    }
}

bool SnakeGame::isOver()
{
    return game_over;
}

void SnakeGame::gameOver()
{
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 21; j++)
        {
            gameBoard[i][j] = '0';
        }
    }
    board.clear();
    board.addAt(10, 17, 'G');
    board.addAt(10, 18, 'a');
    board.addAt(10, 19, 'm');
    board.addAt(10, 20, 'e');
    board.addAt(10, 22, 'O');
    board.addAt(10, 23, 'v');
    board.addAt(10, 24, 'e');
    board.addAt(10, 25, 'r');
    board.addAt(10, 26, '!');

    updateRank();
}

void SnakeGame::setDirection(DIRECTION d)
{
    direction = d;
}

void SnakeGame::setTimeScore()
{
    score->second++;
}

int SnakeGame::getTimeScore() {{
        return score->second;
}}

void SnakeGame::setInnerWall()
{
    srand(time(NULL));
    int pick = rand() % 3 + 1;
    if (pick == 1)
    {
    }
    else if (pick == 2)
    {
        for (int x = 8; x < 15; x++)
        {
            gameBoard[11][x] = '1';
        }
        for (int y = 11; y > 4; y--)
        {
            gameBoard[y][15] = '1';
        }
    }
    else
    {
        for (int x = 5; x < 10; x++)
        {
            gameBoard[4][x] = '1';
        }
        for (int y = 5; y < 10; y++)
        {
            gameBoard[y][5] = '1';
        }
        for (int y = 11; y < 16; y++)
        {
            gameBoard[y][12] = '1';
        }
        for (int x = 12; x < 16; x++)
        {
            gameBoard[13][x] = '1';
        }
    }
}

bool SnakeGame::makeGateCheck(int num)
{
    int gateY = wallYPosition[num];
    int gateX = wallXPosition[num];
    if (((gateY == 0) || (gateY == 20)) || ((gateX == 0) || (gateX == 20)))
    {
        //사이드의 벽이면
        return true;
    }
    else
    {
        //사이드의 벽이 아니면 (inner 벽)
        char leftPoint = gameBoard[gateY][gateX - 1];
        char rightPoint = gameBoard[gateY][gateX + 1];
        char upPoint = gameBoard[gateY - 1][gateX];
        char downPoint = gameBoard[gateY + 1][gateX];
        if (((leftPoint != '9') && (rightPoint != '9')) && ((upPoint != '9') && (downPoint != '9')))
        {
            //상하좌우에 게이트가 없어야함
            if (((leftPoint != '1') && (rightPoint != '1')) && ((upPoint != '1') && (downPoint != '1')))
            {
                //상하좌우가 비어있으면
                return true;
            }
            else if (((leftPoint != '1') && (rightPoint != '1')) && ((upPoint == '1') && (downPoint == '1')))
            {
                //좌우만 비어져있을경우
                return true;
            }
            else if (((leftPoint == '1') && (rightPoint == '1')) && ((upPoint != '1') && (downPoint != '1')))
            {
                //상하만 비어있을 경우
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    //혹시 모르니깐
    return false;
}

void SnakeGame::makeGate()
{
    // 상하가 비어있거나, 좌우가 비어있거나 모두 비어있거나, 사이드의 벽이거나
    //일단 9를 게이트로 해주었음
    int peek1, peek2;
    while (true)
    {
        peek1 = rand() % wallYPosition.size();
        if (makeGateCheck(peek1))
        {
            gate1Y = wallYPosition[peek1];
            gate1X = wallXPosition[peek1];
            break;
        }
    }
    while (true)
    {
        peek2 = rand() % wallYPosition.size();
        if ((peek1 != peek2) && (makeGateCheck(peek2)))
        {
            gate2Y = wallYPosition[peek2];
            gate2X = wallXPosition[peek2];
            break;
        }
    }
    gameBoard[gate1Y][gate1X] = '9';
    gameBoard[gate2Y][gate2X] = '9';
}

void SnakeGame::clearGate(){

    if(gate1Y == -1){
        return;
    }
    gameBoard[gate1Y][gate1X] = '1';
    gameBoard[gate2Y][gate2X] = '1';

}

bool SnakeGame::getEatTimeItem(){
    return eatTimeItem;
}

void SnakeGame::setEatTimeItem(bool status){
    eatTimeItem = status;
}

void SnakeGame::clearTimeItem(){
    if((tItemY!=-1)&&(tItemX!=-1)){
        gameBoard[tItemY][tItemX] = '0';
    }  
}

void SnakeGame::makeTimeItem(){
    int y = rand()%18+1;
    int x = rand()%18+1;
    while(true){
        char refer = gameBoard[y][x];
        if (refer=='0'){
            tItemY = y;
            tItemX = x;
            gameBoard[tItemY][tItemX] = 'T';
            break;
        } else{
            y = rand()%18+1;
            x = rand()%18+1;
        }
    }
}

int SnakeGame::getDelayTime(){
    return delayTime;
}
void SnakeGame::minusDelayTime(){
    if(!(delayTime-1 == 0)){
        delayTime -= 2;
    }
}
void SnakeGame::setOriginDelayTime(){
    delayTime = 5;
}

void SnakeGame::setHalfDelay(int time){
    halfdelay(time);
}

bool SnakeGame::getEatStartStatus(){
    return eatStartStatus;
}

void SnakeGame::setEatStartStatus(bool status){
    eatStartStatus = status;
}

void SnakeGame::updateRank() 
{
    rank->update(score);
}
