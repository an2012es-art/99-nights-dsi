
#include <nds.h>
#include <stdio.h>
#include <stdbool.h>

#define W 20
#define H 12

static const char *map[H] = {
 "####################",
 "#..T....#.....T....#",
 "#.......#..........#",
 "#..##.......T......#",
 "#..............##..#",
 "#....T.............#",
 "#..........##......#",
 "#..C...............#",
 "#........T.........#",
 "#......##......C...#",
 "#....C.............#",
 "####################"
};

int main(void) {
    consoleDemoInit();

    int x = 2, y = 2;
    int wood = 0, children = 0;
    int day = 1, time = 0;
    int fire = 0, shelter = 0, weapon = 0, lamp = 1;
    int mx = 17, my = 8, alive = 1;
    char world[H][W + 1];
    char message[64] = "Find wood and rescue the children!";

    for (int j = 0; j < H; j++) {
        for (int i = 0; i < W; i++)
            world[j][i] = map[j][i];
        world[j][W] = '\0';
    }

    while (1) {
        scanKeys();
        int k = keysDownRepeat();

        if (!alive && (k & KEY_START)) {
            x = 2; y = 2; wood = 0; children = 0;
            day = 1; time = 0; fire = 0;
            shelter = 0; weapon = 0; lamp = 1;
            mx = 17; my = 8; alive = 1;
            for (int j = 0; j < H; j++)
                for (int i = 0; i < W; i++)
                    world[j][i] = map[j][i];
            snprintf(message, sizeof(message), "New day. Find the children!");
        }

        if (alive) {
            int nx = x, ny = y;
            if (k & KEY_UP) ny--;
            if (k & KEY_DOWN) ny++;
            if (k & KEY_LEFT) nx--;
            if (k & KEY_RIGHT) nx++;

            if (nx > 0 && nx < W-1 && ny > 0 && ny < H-1
                && world[ny][nx] != '#') {
                x = nx; y = ny;
            }

            if (k & KEY_A) {
                if (world[y][x] == 'T') {
                    world[y][x] = '.';
                    wood += 2;
                    snprintf(message, sizeof(message), "Chopped tree: +2 wood");
                } else if (world[y][x] == 'C') {
                    world[y][x] = '.';
                    children++;
                    snprintf(message, sizeof(message), "Child rescued!");
                } else {
                    snprintf(message, sizeof(message), "Nothing here.");
                }
            }

            if ((k & KEY_L) && !fire && wood >= 3) {
                wood -= 3; fire = 1;
                snprintf(message, sizeof(message), "Campfire lit!");
            }
            if ((k & KEY_Y) && !shelter && wood >= 4) {
                wood -= 4; shelter = 1;
                snprintf(message, sizeof(message), "Shelter built!");
            }
            if ((k & KEY_B) && !weapon && wood >= 2) {
                wood -= 2; weapon = 1;
                snprintf(message, sizeof(message), "Wooden spear crafted!");
            }
            if (k & KEY_X) {
                lamp = !lamp;
                snprintf(message, sizeof(message), "Flashlight toggled.");
            }

            time++;
            if (time >= 100) {
                time = 0;
                day++;
                mx = 17; my = 8;
            }

            /* The monster only moves at night. */
            if (time >= 50) {
                if (mx < x) mx++;
                else if (mx > x) mx--;
                else if (my < y) my++;
                else if (my > y) my--;

                if (mx == x && my == y) {
                    if (weapon) {
                        weapon = 0;
                        mx = 17; my = 8;
                        snprintf(message, sizeof(message),
                                 "Spear scared monster away!");
                    } else if (fire && shelter) {
                        mx = 17; my = 8;
                        snprintf(message, sizeof(message),
                                 "Fire and shelter saved you!");
                    } else {
                        alive = 0;
                        snprintf(message, sizeof(message),
                                 "Caught by monster! START to retry.");
                    }
                }
            }
        }

        consoleClear();
        iprintf("  99 NIGHTS - FOREST SURVIVAL\n");
        iprintf("Day:%d  %s  Wood:%d  Kids:%d/3\n",
                day, time < 50 ? "DAY" : "NIGHT", wood, children);
        iprintf("Fire:%s Shelter:%s Lamp:%s\n",
                fire ? "ON" : "OFF",
                shelter ? "YES" : "NO",
                lamp ? "ON" : "OFF");
        iprintf("D-pad: Move  A: Chop/Rescue\n");
        iprintf("L: Fire(3 wood) Y: Shelter(4)\n");
        iprintf("B: Spear(2 wood) X: Lamp\n\n");

        for (int j = 0; j < H; j++) {
            for (int i = 0; i < W; i++) {
                if (i == x && j == y) iprintf("@");
                else if (time >= 50 && i == mx && j == my)
                    iprintf("M");
                else iprintf("%c", world[j][i]);
            }
            iprintf("\n");
        }
        iprintf("\n%s\n", message);
        if (children == 3)
            iprintf("All children rescued! Keep surviving!\n");

        swiWaitForVBlank();
    }
    return 0;
}
