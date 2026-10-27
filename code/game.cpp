#include "headers.h"
#include "tile.h"
#include "superblock.h"
#include "controls.h"
#include "game.h"

Game::Game()
{
    level = 2;
    number_of_steps = 0;
    gameover = false;
    won = false;
}

int Game::check()
{
    if(superblock.is_split)
    {
        for(int k=0; k<2; k++)
        {
            int cx = superblock.cube_ix[k], cy = superblock.cube_iy[k];
            if(!stage.has_tile(cx, cy))
            {
                gameover = true;
                return 0;
            }
        }
        return -1;
    }

    int sx = superblock.ix, sy = superblock.iy;
    int tx = stage.tx, ty = stage.ty;
    int along = superblock.along;

    // has_tile() kiểm tra cả biên lưới: ra ngoài lưới = rơi (bản gốc truy cập ngoài mảng).
    // Luật này được chép y hệt trong ai/problem.py (BloxorzProblem.check).
    if(!stage.has_tile(sx, sy))
    {
        if(!(sx==tx && sy==ty))
            gameover = true;
        else if(along==Z)
            won = true;
    }
    else if(along==Z)
    {
        if(stage.tiles[sx][sy]->get_tile_type()==fragile)
            gameover = true;
    }

    if(along == X && !stage.has_tile(sx+1, sy))
        if(!(sx+1==tx && sy==ty))
            gameover = true;
    if(along == Y && !stage.has_tile(sx, sy+1))
        if(!(sx==tx && sy+1==ty))
            gameover = true;

    if(gameover)return 0;
    else if(won)return 1;
    else return -1;
}

void Game::check_after_step()
{
    number_of_steps++;
    int p = check();
    if(p!=-1)return;

    if(superblock.is_split)
    {
        int cx = superblock.cube_ix[superblock.active_cube];
        int cy = superblock.cube_iy[superblock.active_cube];
        if(stage.has_tile(cx, cy) && stage.tiles[cx][cy]->get_tile_type()==normal_bridge)
            stage.normal_bridge_toggle(cx, cy);

        if(superblock.check_recombine())
        {
            int sx = superblock.ix, sy = superblock.iy;
            int along = superblock.along;
            if(stage.has_tile(sx, sy) && stage.tiles[sx][sy]->get_tile_type()==normal_bridge)
                stage.normal_bridge_toggle(sx, sy);
            if(along==X && stage.has_tile(sx+1, sy) && stage.tiles[sx+1][sy]->get_tile_type()==normal_bridge)
                stage.normal_bridge_toggle(sx+1, sy);
            if(along==Y && stage.has_tile(sx, sy+1) && stage.tiles[sx][sy+1]->get_tile_type()==normal_bridge)
                stage.normal_bridge_toggle(sx, sy+1);
        }
        return;
    }

    int sx = superblock.ix, sy = superblock.iy;
    int along = superblock.along;

    if(along==Z && stage.is_split_switch(sx, sy))
    {
        auto tg = stage.get_split_targets(sx, sy);
        superblock.split(tg.first.first, tg.first.second, tg.second.first, tg.second.second);
        check();
        return;
    }

    if(stage.has_tile(sx, sy) && stage.tiles[sx][sy]->get_tile_type()==normal_bridge)
        stage.normal_bridge_toggle(sx, sy);

    if(along==X && stage.has_tile(sx+1, sy) && stage.tiles[sx+1][sy]->get_tile_type()==normal_bridge)
        stage.normal_bridge_toggle(sx+1, sy);

    if(along==Y && stage.has_tile(sx, sy+1) && stage.tiles[sx][sy+1]->get_tile_type()==normal_bridge)
        stage.normal_bridge_toggle(sx, sy+1);

    if(along==Z && stage.has_tile(sx, sy) && stage.tiles[sx][sy]->get_tile_type()==heavy_bridge)
        stage.heavy_bridge_toggle(sx, sy);
}

bool Game::is_gameover()
{
    return gameover;
}
bool Game::is_won()
{
    return won;
}

void Game::initialize(int level)
{
    // Gọi lại được để chơi lại / đổi màn: đặt lại trạng thái ván rồi nạp màn.
    this->level = level;
    number_of_steps = 0;
    gameover = false;
    won = false;
    string name;

    name = string("data/") + to_string(level) + string(".data");
    stage.initialize(name);
    superblock.initialize(
                    stage.get_initial_pos(),
                    glm::vec3(2*tile_side_length, tile_side_length, tile_side_length),
                    colors[BLOCK_COLOR],
                    stage.get_along()
                );
}

void Game::gameover_fun()
{
    cout << "GAME OVER" << endl;
    cout << "Steps : " << number_of_steps << endl;
    //stage.~Floor();
    //superblock.~SuperBlock();
    //initialize();
}
void Game::won_fun()
{
    cout << "WON" << endl;
    cout << "Steps : " << number_of_steps << endl;
    //stage.~Floor();
    //superblock.~SuperBlock();
    //if(level!=1)level++;
    //initialize();
}
int Game::get_level()
{
    return level;
}
int Game::get_steps()
{
    return number_of_steps;
}
