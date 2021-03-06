#include<iostream>
#include<fstream>
#include<iomanip>
#include<sstream>
#include<string>
#include<cstdlib>
#include<cassert>
#include "world_type.h"
using namespace std;
#define NUM_ORDER 9
//haha
void display(grid_t x) {
    for (int i = 0; i < x.height; ++i) {
        for (int j = 0; j < x.width; ++j) {
            string t = "";
            if (x.squares[i][j] == nullptr) {
                t = "____";
            }
            else {
                creature_t tmp = *x.squares[i][j];
                t = tmp.species->name.substr(0, 2);
                t += "_";
                t += directShortName[tmp.direction];
            }
            cout << t << " ";
        }
        cout << endl;
    }
}

bool dohill(creature_t &x, const grid_t & grid) {
    int px = x.location.r;
    int py = x.location.c;
    if (x.ability[FLY]) return 0;
    if (grid.terrain[px][py] == HILL) {
        if (!x.hillActive) {
            x.hillActive = 1;
            return 1;
        }
    }
    return 0;
}

bool outofbound(int nx, int ny, const grid_t & grid) {
    if (nx < 0 || ny < 0 || nx >= grid.height || ny >= grid.width) return 1;
    else return 0;
}

void act_hop(creature_t &x, grid_t &grid) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    x.programID = (x.programID + 1) % x.species->programSize;
    int px = x.location.r;
    int py = x.location.c;
    int dx[4] = {0, 1, 0, -1}, dy[4] = {1, 0, -1, 0};
    int nx = px + dx[x.direction], ny = py + dy[x.direction];
    if (outofbound(nx, ny, grid)) return;
    if (grid.squares[nx][ny] != nullptr) return;
    if (grid.terrain[nx][ny] == LAKE && x.ability[FLY] == 0) return;

    x.location.r = nx;
    x.location.c = ny;
    grid.squares[nx][ny] = grid.squares[px][py];
    grid.squares[px][py] = nullptr;
}

void act_left(creature_t &x, grid_t &grid) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    x.programID = (x.programID + 1) % x.species->programSize;
    int t = (int)(x.direction) + 3;
    t %= 4;
    x.direction = direction_t(t);
}

void act_right(creature_t &x, grid_t &grid) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    x.programID = (x.programID + 1) % x.species->programSize;
    int t = (int)(x.direction) + 1;
    t %= 4;
    x.direction = direction_t(t);
}

void act_infect(creature_t &x, grid_t &grid) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    x.programID = (x.programID + 1) % x.species->programSize;
    int px = x.location.r, py = x.location.c;
    int dx[4] = {0, 1, 0, -1}, dy[4] = {1, 0, -1, 0};
    int nx = px + dx[x.direction], ny = py + dy[x.direction];
    if (x.ability[ARCH]) {
        nx = px, ny = py;
        while (1) {
            nx += dx[x.direction];
            ny += dy[x.direction];
            if (outofbound(nx, ny, grid)) return;
            if (grid.squares[nx][ny] == nullptr) continue;
            if (grid.squares[nx][ny]->species == grid.squares[px][py]->species) continue;
            creature_t *t = grid.squares[px][py];
            t->species = grid.squares[nx][nx]->species;
            t->programID = 0;
        }
    } else {
        if (outofbound(nx, ny, grid)) return;
        if (grid.squares[nx][ny] == nullptr) return;
        if (grid.terrain[nx][ny] == FOREST) return;
        if (grid.squares[nx][ny]->species == grid.squares[px][py]->species) return;
        
        //grid.squares[px][py]->species = grid.squares[px][py]->species;
        creature_t *t = grid.squares[nx][ny];
        // cout << t->species->name << endl;
        t->species = grid.squares[px][py]->species;
        // cout << t->species->name << endl;
        t->programID = 0;
    }
}

void act_ifempty(creature_t &x, grid_t &grid, int to) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    int px = x.location.r, py = x.location.c;
    int dx[4] = {0, 1, 0, -1}, dy[4] = {1, 0, -1, 0};
    int nx = px + dx[x.direction], ny = py + dy[x.direction];
    if (outofbound(nx, ny, grid) || (grid.squares[nx][ny] != nullptr && grid.terrain[nx][ny] != FOREST))
        x.programID = (x.programID + 1) % x.species->programSize;
    else x.programID = to;
}
void act_ifenemy(creature_t &x, grid_t &grid, int to) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    int px = x.location.r, py = x.location.c;
    int dx[4] = {0, 1, 0, -1}, dy[4] = {1, 0, -1, 0};
    int nx = px + dx[x.direction], ny = py + dy[x.direction];
    // cout << x.species->programSize << endl;
    if (outofbound(nx, ny, grid) || grid.squares[nx][ny] == nullptr || grid.squares[nx][ny]->species == grid.squares[px][py]->species || grid.terrain[nx][ny] == FOREST) {
        x.programID = (x.programID + 1) % x.species->programSize;
        // cout << x.programID << endl;
        // cout << "fail" << endl;
    }
    else x.programID = to;
}
void act_ifsame(creature_t &x, grid_t &grid, int to) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    int px = x.location.r, py = x.location.c;
    int dx[4] = {0, 1, 0, -1}, dy[4] = {1, 0, -1, 0};
    int nx = px + dx[x.direction], ny = py + dy[x.direction];
    if (outofbound(nx, ny, grid) || grid.squares[nx][ny] == nullptr || grid.squares[nx][ny]->species != grid.squares[px][py]->species || grid.terrain[nx][ny] == FOREST)
        x.programID = (x.programID + 1) % x.species->programSize;
    else x.programID = to;
}
void act_ifwall(creature_t &x, grid_t &grid, int to) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    int px = x.location.r, py = x.location.c;
    int dx[4] = {0, 1, 0, -1}, dy[4] = {1, 0, -1, 0};
    int nx = px + dx[x.direction], ny = py + dy[x.direction];
    if (outofbound(nx, ny, grid) || (x.ability[FLY] == 0 && grid.terrain[nx][ny] == LAKE))
        x.programID = to;
    else x.programID = (x.programID + 1) % x.species->programSize;
}
void act_go(creature_t &x, grid_t &grid, int to) {
    if (dohill(x, grid)) return;
    x.hillActive = 0;
    x.programID = to;
}
string parseint(int x) {
    if (x == 0) return "0";
    else {
        string s = "";
        while (x) {
            char ch = x % 10 + '0';
            x /= 10;
            s = ch + s;
        }
        return s;
    }
}
void act(creature_t &x, grid_t &grid, bool verbose) {
    int id;
    bool ok = 0;
    string msg = "Creature (" + x.species->name + " " + directName[x.direction] + " ";
    msg += parseint(x.location.r) + " " + parseint(x.location.c) + ") takes action:";
    if (verbose) msg += '\n'; else msg += ' ';
    while (!ok) {
        id = x.programID;
        switch (x.species->program[x.programID].op) {
            case HOP: 
                act_hop(x, grid);
                if (!verbose) msg += "hop\n";
                else msg += "Instruction " + parseint(id + 1) + ": hop\n";
                ok = 1;
                break;
            case LEFT:
                act_left(x, grid);
                if (!verbose) msg += "left\n";
                else msg += "Instruction " + parseint(id + 1) + ": left\n";
                ok = 1;
                break;
            case RIGHT:
                act_right(x, grid);
                if (!verbose) msg += "right\n";
                else msg += "Instruction " + parseint(id + 1) + ": right\n";
                ok = 1;
                break;
            case INFECT:
                act_infect(x, grid);
                if (!verbose) msg += "infect\n";
                else msg += "Instruction " + parseint(id + 1) + ": infect\n";
                ok = 1;
                break;
            case IFEMPTY:
                act_ifempty(x, grid, (int)x.species->program[id].address - 1);
                if (verbose)
                    msg += "Instruction " + parseint(id + 1) + ": ifempty " + parseint((int)x.species->program[id].address) + "\n";
                break;
            case IFENEMY:
                act_ifenemy(x, grid, (int)x.species->program[id].address - 1);
                if (verbose)
                    msg += "Instruction " + parseint(id + 1) + ": ifenemy " + parseint((int)x.species->program[id].address) + "\n";
                break;
            case IFSAME:
                act_ifsame(x, grid, (int)x.species->program[id].address - 1);
                if (verbose)
                    msg += "Instruction " + parseint(id + 1) + ": ifsame " + parseint((int)x.species->program[id].address) + "\n";
                break;
            case IFWALL:
                act_ifwall(x, grid, (int)x.species->program[id].address - 1);
                if (verbose)
                    msg += "Instruction " + parseint(id + 1) + ": ifwall " + parseint((int)x.species->program[id].address) + "\n";
                break;
            case GO:
                act_go(x, grid, (int)x.species->program[id].address - 1);
                if (verbose)
                    msg += "Instruction " + parseint(id + 1) + ": go " + parseint((int)x.species->program[id].address) + "\n";
                break;
            default:
                assert(0);
                break;
        }
    }
    cout << msg;
    if (verbose) display(grid);
}

void simulate(world_t &x, int lmt, bool verbose) {
    cout << "Initial state" << endl;
    display(x.grid);
    for (int rd = 1; rd <= lmt; ++rd) {
        cout << "Round " + parseint(rd) + "\n";
        for (int i = 0; i < x.numCreatures; ++i) {
            act(x.creatures[i], x.grid, verbose);
        }
        if (!verbose) display(x.grid);
    }
}

void test(creature_t * x) {
    cout << x->location.r << endl;
    cout << x->location.c << endl;
    cout << x->direction << endl;
    cout << x->ability[0] << " / " << x->ability[1] << endl;
    cout << (x->species)->name << endl;
}

int main(int argc, char *argv[]) {
    world_t WD;
    string map[111];
    //cout << argc << endl;
    //printf("%s\n", argv[0]);

    ifstream ifs_species_summary;
    string dir = argv[1];
    dir = "./Tests/" + dir;
    //cout << dir << endl;
    ifs_species_summary.open(dir);
    //cout << ifs_species_summary.good() << endl;
    //cout << endl;
    // ifs_species_summary

    string str, dirstr;
    int cc = -1;
    string crea_str[111];
    while (getline(ifs_species_summary, str)) {
        if (str == "\n") continue;
        if (cc == -1) {
            dirstr = str;
            ++cc;
        } else crea_str[cc++] = str;
    }
    ifstream ifs_prog_crea[111];
    for (int i = 0; i < cc; ++i) {
        dir = "./Tests/" + dirstr + "/" + crea_str[i];
        //cout << dir << endl;
        ifs_prog_crea[i].open(dir);
        //cout << ifs_prog_crea[i].good() << endl;
        WD.species[i].name = crea_str[i];
        //cout << WD.species[i].name << endl;
        string tmpstr;
        int cprog = 0;
        char ins[111];
        while (getline(ifs_prog_crea[i], tmpstr)) {
            *ins = 0;
            int x = 0;
            sscanf(tmpstr.c_str(), "%s%d", ins, &x);
            if (*ins == 0) break;
            //cout << ins << endl;
            /*cout << ins[1] << endl;
            if (ins[1] == 'f') {//if condition
                sscanf(tmpstr.c_str(), "%d", &x);
                cout << tmpstr.c_str() << endl;
            }*/
            //cout << cprog << " : " << ins << endl;
            //if (x) cout << "ifcond | go x : " << x << endl;
            instruction_t tmpins;
            if (x) tmpins.address = x; else tmpins.address = 0;

            for (int j = 0; j < NUM_ORDER; ++j)
                if ((string)ins == opName[j])
                  tmpins.op = opcode_t(j);
            WD.species[i].program[cprog] = tmpins;
            ++cprog;
        }
        //cout << "cprog : " << cprog << endl;
        WD.species[i].programSize = cprog;
        /*for (int j = 0; j < WD.species[i].programSize; ++j) {
            cout << WD.species[i].program[j].op << endl;
            if (WD.species[i].program[j].address)
                cout << "address is " << WD.species[i].program[j].address << endl;
        }
        cout << endl;*/

        //cout << endl;
    }
    //cout << "total : " << cc << endl;
    WD.numSpecies = cc;

    //cout << "Creatures finished" << endl << endl << "/////////////////////" << endl << endl;
    //cout << "begin to read world:" << endl << endl;

    ifstream ifs_world;
    ifs_world.open("./Tests/world-tests/" + string(argv[2]));
    //cout << ifs_world.good() << endl;

    int r = 0, c = 0;
    grid_t &grid = WD.grid;
    getline(ifs_world, str);
    sscanf(str.c_str(), "%d", &r);

    getline(ifs_world, str);
    sscanf(str.c_str(), "%d", &c);

    for (int i = 0; i < r; ++i) {
        getline(ifs_world, map[i]);
        //cout << map[i] << endl;
    }
    grid.height = r;
    grid.width = c;
    for (int i = 0; i < r; ++i) 
        for (int j = 0; j < c; ++j) grid.squares[i][j] = nullptr;
    WD.numCreatures = 0;
    string crea_info[111];
    while (getline(ifs_world, str)) {
        //if (str == "\n") continue;
        crea_info[WD.numCreatures++] = str;
        //cout << "here" << endl;
        //cout << str + "~~~" << endl;
    }
    for (int i = 0; i < WD.numCreatures; ++i) {
        char name[111], dir[111], a1[5], a2[5];
        int x, y;
        a1[0] = a2[0] = 0;
        sscanf(crea_info[i].c_str(), "%s%s%d%d%s%s", name, dir, &x, &y, a1, a2);
        //cout << "nxt " << endl;
        creature_t &tmpcrea = WD.creatures[i]; 
        tmpcrea.location.r = x;
        tmpcrea.location.c = y;
        tmpcrea.ability[0] = tmpcrea.ability[1] = 0;
        if (a1[0]) {
            if (a1[0] == 'f') 
                tmpcrea.ability[0] = 1;
            else tmpcrea.ability[1] = 1;
        }
        if (a2[0]) {
            if (a2[0] == 'f')
                tmpcrea.ability[0] = 1; 
            else tmpcrea.ability[1] = 1;
        }
        tmpcrea.programID = 0;
        for (int j = 0; j < 4; ++j)
            if ((string)dir == directName[j])
                tmpcrea.direction = direction_t(j);

        for (int j = 0; j < WD.numSpecies; ++j)
            if ((string)name == WD.species[j].name)
                tmpcrea.species = &WD.species[j];
        //test(&WD.creatures[i]);
        grid.squares[x][y] = &WD.creatures[i];
    }
    //cout << "there" << endl;
    for (int i = 0; i < r; ++i)
        for (int j = 0; j < c; ++j) {
            for (int k = 0; k < 4; ++k) {
                if (map[i][j] == terrainShortName[k][0]) {
                    grid.terrain[i][j] = terrain_t(k);
                }
            }
        }
    simulate(WD, 20, 0);
    return 0;
}