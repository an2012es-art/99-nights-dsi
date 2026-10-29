#include <nds.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define SCREEN_W 256
#define SCREEN_H 192
#define MAP_W 16
#define MAP_H 12
#define TILE 16

typedef unsigned short u16;
#define RGB15(r,g,b) ((r) | ((g)<<5) | ((b)<<10) | BIT(15))

static u16 *fb = (u16*)0x06000000;

static const u16 C_BLACK = RGB15(1,1,2);
static const u16 C_DARK  = RGB15(3,8,5);
static const u16 C_GRASS = RGB15(4,15,6);
static const u16 C_GRASS2= RGB15(5,19,8);
static const u16 C_TREE  = RGB15(2,10,3);
static const u16 C_BROWN = RGB15(15,8,3);
static const u16 C_WOOD  = RGB15(22,13,5);
static const u16 C_SKIN  = RGB15(26,17,12);
static const u16 C_BLUE  = RGB15(5,12,27);
static const u16 C_RED   = RGB15(26,3,3);
static const u16 C_WHITE = RGB15(29,29,27);
static const u16 C_YELL  = RGB15(30,22,3);
static const u16 C_ORANGE= RGB15(31,10,1);
static const u16 C_PURPLE= RGB15(16,4,22);
static const u16 C_GREY  = RGB15(13,15,15);

static int px=2, py=2, wood=0, rescued=0, day=1, ticks=0;
static int fire=0, shelter=0, fence=0, weapon=0, lamp=1, alive=1;
static int monsterX=13, monsterY=8;
static char world[MAP_H][MAP_W+1] = {
 "################",
 "#..T....#.....C#",
 "#.......#......#",
 "#..##.......T..#",
 "#..............#",
 "#....T.........#",
 "#..........##..#",
 "#..C...........#",
 "#........T.....#",
 "#......##....C.#",
 "#..............#",
 "################"
};
static char message[72]="Chop trees, rescue children, survive the night!";

static void rect(int x,int y,int w,int h,u16 c) {
    if(x<0||y<0||x+w>SCREEN_W||y+h>SCREEN_H) return;
    for(int yy=y; yy<y+h; yy++)
        for(int xx=x; xx<x+w; xx++) fb[yy*SCREEN_W+xx]=c;
}
static void pixel(int x,int y,u16 c) {
    if(x>=0&&x<SCREEN_W&&y>=0&&y<SCREEN_H) fb[y*SCREEN_W+x]=c;
}
static void clear_screen(u16 c) {
    for(int i=0;i<SCREEN_W*SCREEN_H;i++) fb[i]=c;
}
static void draw_tree(int x,int y) {
    int sx=x*TILE, sy=y*TILE;
    rect(sx+5,sy+9,6,7,C_BROWN);
    rect(sx+2,sy+2,12,9,C_TREE);
    rect(sx+4,sy,8,12,C_GRASS2);
    pixel(sx+4,sy+4,C_GRASS); pixel(sx+11,sy+3,C_GRASS);
}
static void draw_child(int x,int y) {
    int sx=x*TILE, sy=y*TILE;
    rect(sx+5,sy+3,6,5,C_SKIN);
    rect(sx+4,sy+2,8,3,C_BROWN);
    rect(sx+4,sy+8,8,6,C_PURPLE);
    rect(sx+3,sy+13,4,3,C_BLUE); rect(sx+9,sy+13,4,3,C_BLUE);
    pixel(sx+6,sy+5,C_BLACK); pixel(sx+10,sy+5,C_BLACK);
}
static void draw_player(int x,int y) {
    int sx=x*TILE, sy=y*TILE;
    /* backpack */
    rect(sx+3,sy+7,3,6,C_BROWN);
    /* head/hair */
    rect(sx+6,sy+2,6,6,C_SKIN);
    rect(sx+5,sy+1,8,3,C_BROWN);
    pixel(sx+7,sy+5,C_BLACK); pixel(sx+10,sy+5,C_BLACK);
    /* jacket and arms */
    rect(sx+5,sy+8,8,6,C_BLUE);
    rect(sx+3,sy+9,2,5,C_SKIN); rect(sx+13,sy+9,2,5,C_SKIN);
    /* legs and boots */
    rect(sx+6,sy+14,3,2,C_GREY); rect(sx+10,sy+14,3,2,C_GREY);
    rect(sx+5,sy+15,4,1,C_BLACK); rect(sx+10,sy+15,4,1,C_BLACK);
    /* lamp glow if enabled */
    if(lamp && ticks>=45) {
        for(int r=1;r<4;r++) {
            pixel(sx+15+r,sy+9,C_YELL);
            pixel(sx+15+r,sy+10,C_YELL);
        }
    }
}
static void draw_monster(int x,int y) {
    int sx=x*TILE, sy=y*TILE;
    /* antler silhouette */
    rect(sx+2,sy+1,2,5,C_GREY); rect(sx+1,sy+1,4,2,C_GREY);
    rect(sx+12,sy+1,2,5,C_GREY); rect(sx+11,sy+1,4,2,C_GREY);
    /* dark body and head */
    rect(sx+4,sy+4,8,8,C_BLACK);
    rect(sx+3,sy+3,10,6,C_DARK);
    rect(sx+5,sy+6,2,2,C_RED); rect(sx+10,sy+6,2,2,C_RED);
    rect(sx+5,sy+12,3,4,C_BLACK); rect(sx+9,sy+12,3,4,C_BLACK);
}
static void draw_fire(int x,int y) {
    int sx=x*TILE, sy=y*TILE;
    rect(sx+3,sy+11,10,3,C_BROWN);
    rect(sx+5,sy+7,6,6,C_ORANGE);
    rect(sx+7,sy+4,3,7,C_YELL);
    pixel(sx+6,sy+8,C_RED); pixel(sx+10,sy+9,C_RED);
}
static void draw_house(int x,int y) {
    int sx=x*TILE, sy=y*TILE;
    rect(sx+2,sy+7,12,8,C_WOOD);
    rect(sx+1,sy+5,14,3,C_BROWN);
    rect(sx+5,sy+10,4,5,C_BLACK);
}
static void draw_world(void) {
    u16 bg = (ticks>=45) ? RGB15(1,2,7) : C_GRASS;
    clear_screen(bg);
    for(int y=0;y<MAP_H;y++) for(int x=0;x<MAP_W;x++) {
        int sx=x*TILE, sy=y*TILE;
        char c=world[y][x];
        if(c=='#') {
            rect(sx,sy,TILE,TILE,C_TREE);
            rect(sx+1,sy+1,14,14,C_DARK);
        } else {
            rect(sx,sy,TILE,TILE, ( (x+y)%2 ) ? C_GRASS : C_GRASS2);
            pixel(sx+2,sy+12,C_GRASS); pixel(sx+13,sy+4,C_GRASS);
            if(c=='T') draw_tree(x,y);
            if(c=='C') draw_child(x,y);
        }
    }
    if(fire) draw_fire(2,2);
    if(shelter) draw_house(3,2);
    if(fence) {
        rect(16,32,2,30,C_WOOD); rect(18,32,2,30,C_WOOD);
        rect(20,32,2,30,C_WOOD); rect(22,32,2,30,C_WOOD);
    }
    if(ticks>=45 && alive) draw_monster(monsterX,monsterY);
    draw_player(px,py);
}
static bool walkable(int x,int y) {
    return x>0 && x<MAP_W-1 && y>0 && y<MAP_H-1 && world[y][x]!='#';
}
static int abs_i(int v){return v<0?-v:v;}
static void interact(void) {
    if(world[py][px]=='T') {
        world[py][px]='.'; wood+=2;
        snprintf(message,sizeof(message),"Chopped tree: +2 wood.");
    } else if(world[py][px]=='C') {
        world[py][px]='.'; rescued++;
        snprintf(message,sizeof(message),"Child rescued! (%d/3)",rescued);
    } else snprintf(message,sizeof(message),"Nothing here. Stand on tree or child.");
}
static void build(void) {
    if(!shelter && wood>=4) { wood-=4; shelter=1; snprintf(message,sizeof(message),"Shelter built!"); }
    else if(!fence && wood>=3) { wood-=3; fence=1; snprintf(message,sizeof(message),"Fence built!"); }
    else snprintf(message,sizeof(message),"Need 4 wood for shelter, then 3 for fence.");
}
static void light_fire(void) {
    if(!fire && wood>=3) { wood-=3; fire=1; snprintf(message,sizeof(message),"Campfire is burning!"); }
    else snprintf(message,sizeof(message), fire?"Campfire already lit.":"Need 3 wood for fire.");
}
static void monster_update(void) {
    if(ticks<45 || !alive) return;
    if(fire && abs_i(px-2)+abs_i(py-2)<=2) return;
    if(fence && px<=5 && py<=5) return;
    if(monsterX<px) monsterX++; else if(monsterX>px) monsterX--;
    if(monsterY<py) monsterY++; else if(monsterY>py) monsterY--;
    if(monsterX==px && monsterY==py) {
        if(weapon) { weapon=0; monsterX=13; monsterY=8; snprintf(message,sizeof(message),"Spear drove it away!"); }
        else { alive=0; snprintf(message,sizeof(message),"Caught! Press START to retry."); }
    }
}
static void draw_ui(void) {
    consoleClear();
    iprintf("99 NIGHTS - PIXEL FOREST\n");
    iprintf("Day %d | %s | Wood %d | Kids %d/3\n",day,ticks<45?"DAY":"NIGHT",wood,rescued);
    iprintf("Fire:%s Shelter:%s Fence:%s\n",fire?"ON":"OFF",shelter?"YES":"NO",fence?"YES":"NO");
    iprintf("Lamp:%s Spear:%s\n",lamp?"ON":"OFF",weapon?"YES":"NO");
    iprintf("D-pad Move A Chop/Rescue\n");
    iprintf("L Fire(3) Y Build B Spear(2)\n");
    iprintf("X Lamp R Status START Retry\n");
    iprintf("%s\n",message);
    if(rescued>=3) iprintf("All children rescued! Survive!\n");
}
int main(void) {
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    REG_DISPCNT = MODE_5_2D | DISPLAY_BG2_ACTIVE;
    BG2_CR = BG_BMP16_256x256;
    consoleDemoInit();
    while(1) {
        scanKeys();
        int k=keysDownRepeat();
        if(!alive && (k&KEY_START)) {
            px=2;py=2;wood=0;rescued=0;day=1;ticks=0;fire=0;shelter=0;fence=0;weapon=0;lamp=1;alive=1;monsterX=13;monsterY=8;
            for(int y=0;y<MAP_H;y++) for(int x=0;x<MAP_W;x++) if(world[y][x]=='t')world[y][x]='T';
            snprintf(message,sizeof(message),"New journey. Rescue the children!");
        }
        if(alive) {
            int nx=px,ny=py;
            if(k&KEY_UP)ny--; if(k&KEY_DOWN)ny++;
            if(k&KEY_LEFT)nx--; if(k&KEY_RIGHT)nx++;
            if(walkable(nx,ny)){px=nx;py=ny;}
            if(k&KEY_A)interact();
            if(k&KEY_Y)build();
            if(k&KEY_L)light_fire();
            if((k&KEY_B)&&!weapon&&wood>=2){wood-=2;weapon=1;snprintf(message,sizeof(message),"Wooden spear crafted!");}
            if(k&KEY_X){lamp=!lamp;snprintf(message,sizeof(message),"Flashlight %s",lamp?"ON":"OFF");}
            if(k&KEY_R)snprintf(message,sizeof(message),rescued==3?"All children rescued!":"Keep searching: %d/3 rescued",rescued);
            ticks++;
            if(ticks>=90){ticks=0;day++;monsterX=13;monsterY=8;}
            if(ticks==45)snprintf(message,sizeof(message),"NIGHT! The monster is hunting!");
            if(ticks==0)snprintf(message,sizeof(message),"Morning. Gather wood and explore.");
            monster_update();
        }
        draw_world();
        draw_ui();
        swiWaitForVBlank();
    }
    return 0;
}
