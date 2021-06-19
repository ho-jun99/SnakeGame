#ifndef __MISSION__
#define __MISSION__
#include <ncurses.h>
class Score;

class Mission
{
public:
    Mission() {}
    Mission(Score *s);

    int check();
    int stage;
private:

    Score *score;
    int checkFlag;
};

class MissionBoard
{
public:
    MissionBoard() {}
    MissionBoard(int height, int width, Mission *m);
    
    void init();
    void addBoarder();
    void clear();
    bool addAtState();
    void refresh();

private:
    WINDOW *mission_board;
    Mission *mission;
};

#endif