
#include <nds.h>
#include <stdlib.h>

#define W 256
#define H 192
#define C(r,g,b) RGB15(r,g,b)

static u16 *screen = (u16*)0x06000000;

int px=120, py=105;
int mx=30, my=35;

int wood=0;
int rescued=0;
int fireFuel=0;
int shelter=0;
int flashlight=0;
int night=0;
int day=1;
int timer=0;
int frame=0;
int gameOver=0;

int campX=120, campY=130;

typedef struct {
    int x,y,alive;
} Object;

Object trees[7]={
    {20,30,1},{55,65,1},{220,35,1},
    {230,125,1},{30,155,1},{180,155,1},
    {75,115,1}
};

Object kids[3]={
    {210,65,1},{35,90,1},{190,120,1}
};

void pixel(int x,int y,u16 color)
{
    if(x>=0 && x<W && y>=0 && y<H)
        screen[y*W+x]=color;
}

void rect(int x,int y,int w,int h,u16 color)
{
    int i,j;
    for(j=0;j<h;j++)
        for(i=0;i<w;i++)
            pixel(x+i,y+j,color);
}

void clearScreen(u16 color)
{
    int i;
    for(i=0;i<W*H;i++)
        screen[i]=color;
}

int nearObject(int x1,int y1,int x2,int y2,int range)
{
    return abs(x1-x2)+abs(y1-y2)<range;
}

void drawTree(int x,int y)
{
    rect(x+7,y+17,7,13,C(12,7,3));
    rect(x+1,y+8,19,14,C(3,12,5));
    rect(x+4,y+2,13,12,C(5,18,7));
    rect(x+7,y-4,8,10,C(7,22,8));
    pixel(x+6,y+10,C(10,25,9));
}

void drawForest(void)
{
    int x,y;

    clearScreen(night ? C(2,4,9):C(7,18,8));

    for(y=0;y<H;y+=12)
        for(x=0;x<W;x+=13)
            if((x*7+y*3)%5==0)
                pixel(x,y,C(10,24,8));

    /* طريق وسط الغابة */
    rect(80,0,95,H,C(14,17,10));

    for(x=0;x<7;x++)
        if(trees[x].alive)
            drawTree(trees[x].x,trees[x].y);

    /* الكوخ */
    rect(170,35,55,40,C(15,8,4));
    rect(164,29,67,10,C(21,11,5));
    rect(188,49,15,26,C(6,4,3));
    rect(176,43,10,10,C(31,22,8));

    /* نار المخيم */
    rect(campX-15,campY+4,30,5,C(12,7,3));

    if(fireFuel>0)
    {
        rect(campX-8,campY-8,16,13,C(31,12,1));
        rect(campX-4,campY-15,8,12,C(31,25,3));
        pixel(campX,campY-18,C(31,31,12));
    }

    /* السياج */
    rect(65,165,125,4,C(19,11,4));
    rect(70,158,4,12,C(23,14,5));
    rect(105,158,4,12,C(23,14,5));
    rect(145,158,4,12,C(23,14,5));
    rect(185,158,4,12,C(23,14,5));
}

void drawPlayer(void)
{
    int y=py+(frame%16<8?0:1);

    /* الظل */
    rect(px-7,y+14,15,3,C(2,2,2));

    /* الأرجل */
    rect(px-6,y+8,5,8,C(4,5,11));
    rect(px+2,y+8,5,8,C(4,5,11));

    /* الجسم */
    rect(px-8,y-4,16,13,C(7,12,22));
    rect(px-10,y,3,9,C(8,12,20));
    rect(px+8,y,3,9,C(8,12,20));

    /* الرأس والشعر */
    rect(px-7,y-16,14,12,C(23,16,11));
    rect(px-8,y-18,16,5,C(5,5,5));
    rect(px-5,y-13,10,6,C(25,19,14));

    pixel(px-4,y-10,C(31,31,31));
    pixel(px+3,y-10,C(31,31,31));

    /* الحقيبة */
    rect(px+7,y-2,4,8,C(18,10,4));

    if(flashlight)
        rect(px+9,y-7,5,4,C(31,30,12));
}

void drawDeer(void)
{
    int bob=frame%12<6?1:0;
    int leg=frame%8<4?3:-3;
    int y=my+bob;

    /* الظل */
    rect(mx-14,y+17,28,4,C(1,2,2));

    /* القرون */
    rect(mx-3,y-31,3,13,C(2,2,3));
    rect(mx-14,y-38,3,11,C(2,2,3));
    rect(mx-21,y-44,3,9,C(2,2,3));
    rect(mx-27,y-47,9,3,C(2,2,3));
    rect(mx-17,y-42,8,3,C(2,2,3));

    rect(mx+1,y-31,3,13,C(2,2,3));
    rect(mx+12,y-38,3,11,C(2,2,3));
    rect(mx+19,y-44,3,9,C(2,2,3));
    rect(mx+18,y-47,9,3,C(2,2,3));
    rect(mx+14,y-42,8,3,C(2,2,3));

    /* الرأس */
    rect(mx-10,y-25,20,17,C(21,10,7));
    rect(mx-6,y-21,12,12,C(25,17,12));

    /* العينان */
    rect(mx-8,y-20,5,5,C(31,31,28));
    rect(mx+3,y-20,5,5,C(31,31,28));
    pixel(mx-6,y-19,C(31,7,2));
    pixel(mx+5,y-19,C(31,7,2));

    /* الخطم والفم */
    rect(mx-4,y-10,8,10,C(12,5,4));
    rect(mx-3,y-9,6,6,C(1,1,1));

    /* الرقبة والجسم */
    rect(mx-6,y-6,12,12,C(18,8,6));
    rect(mx-12,y+2,24,12,C(22,10,6));

    /* الأرجل */
    rect(mx-10,y+12+leg,5,9,C(9,5,4));
    rect(mx+5,y+12-leg,5,9,C(9,5,4));
    rect(mx-10,y+19+leg,6,4,C(2,2,2));
    rect(mx+5,y+19-leg,6,4,C(2,2,2));
}

void drawKids(void)
{
    int i;

    for(i=0;i<3;i++)
    {
        if(kids[i].alive)
        {
            int x=kids[i].x;
            int y=kids[i].y;

            rect(x-5,y-10,10,9,C(23,17,12));
            rect(x-6,y,12,9,C(10,18,26));
            rect(x-5,y+8,4,6,C(3,4,6));
            rect(x+2,y+8,4,6,C(3,4,6));
        }
    }
}

void drawShelter(void)
{
    if(!shelter) return;

    rect(25,30,35,22,C(18,10,5));
    rect(21,25,43,7,C(25,13,5));
    rect(38,39,9,13,C(5,5,4));
}

void drawNumber(int x,int y,int n)
{
    /* أرقام صغيرة باستخدام 3x5 */
    const unsigned char digits[10][5]={
        {7,5,5,5,7},{2,6,2,2,7},
        {7,1,7,4,7},{7,1,7,1,7},
        {5,5,7,1,1},{7,4,7,1,7},
        {7,4,7,5,7},{7,1,1,1,1},
        {7,5,7,5,7},{7,5,7,1,7}
    };

    if(n<0)n=0;
    if(n>9)n=9;

    int a,b;

    for(a=0;a<5;a++)
        for(b=0;b<3;b++)
            if(digits[n][a]&(4>>b))
                pixel(x+b,y+a,C(31,31,31));
}

void drawHUD(void)
{
    rect(3,3,250,23,C(1,3,4));

    /* الخشب */
    rect(8,9,7,8,C(24,13,5));
    drawNumber(19,9,wood);

    /* الأطفال */
    rect(55,9,8,8,C(24,19,12));
    drawNumber(68,9,rescued);

    /* اليوم */
    drawNumber(220,9,day/10);
    drawNumber(226,9,day%10);

    if(night)
        rect(245,9,5,5,C(28,28,31));
    else
        rect(245,9,5,5,C(31,25,5));
}

void moveDeer(void)
{
    if(!night) return;

    int dx=px-mx;
    int dy=py-my;
    int distance=abs(dx)+abs(dy);

    int speed=1;

    if(distance<85)
        speed=2;

    /* النار تبعد الغزال */
    if(fireFuel>0 && distance<55)
        return;

    if(dx>0) mx+=speed;
    if(dx<0) mx-=speed;

    if(dy>0) my+=speed;
    if(dy<0) my-=speed;

    if(abs(mx-px)<13 && abs(my-py)<13)
        gameOver=1;
}

void collectWood(void)
{
    int i;

    for(i=0;i<7;i++)
    {
        if(trees[i].alive &&
           nearObject(px,py,trees[i].x+10,trees[i].y+10,24))
        {
            trees[i].alive=0;
            wood+=3;
            return;
        }
    }
}

void rescueChild(void)
{
    int i;

    for(i=0;i<3;i++)
    {
        if(kids[i].alive &&
           nearObject(px,py,kids[i].x,kids[i].y,22))
        {
            kids[i].alive=0;
            rescued++;
            return;
        }
    }
}

void addFire(void)
{
    if(nearObject(px,py,campX,campY,35) && wood>0)
    {
        wood--;
        fireFuel+=600;
    }
}

void buildShelter(void)
{
    if(nearObject(px,py,campX,campY,45) &&
       wood>=4 && !shelter)
    {
        wood-=4;
        shelter=1;
    }
}

int main(void)
{
    videoSetMode(MODE_FB0);
    vramSetBankA(VRAM_A_LCD);

    while(1)
    {
        scanKeys();

        int held=keysHeld();
        int down=keysDown();

        if(!gameOver)
        {
            /* حركة اللاعب */
            if(held&KEY_LEFT) px-=2;
            if(held&KEY_RIGHT) px+=2;
            if(held&KEY_UP) py-=2;
            if(held&KEY_DOWN) py+=2;

            if(px<8)px=8;
            if(px>248)px=248;
            if(py<12)py=12;
            if(py>178)py=178;

            /* A: قطع الأشجار */
            if(down&KEY_A)
                collectWood();

            /* B: إضافة الخشب للنار */
            if(down&KEY_B)
                addFire();

            /* X: إنقاذ الأطفال */
            if(down&KEY_X)
                rescueChild();

            /* Y: بناء المأوى */
            if(down&KEY_Y)
                buildShelter();

            /* L: المصباح */
            if(down&KEY_L)
                flashlight=!flashlight;

            /* دورة الليل والنهار */
            timer++;

            if(timer>=1800)
            {
                timer=0;
                night=!night;

                if(!night)
                    day++;
            }

            /* استهلاك وقود النار */
            if(fireFuel>0)
                fireFuel--;

            /* حركة الغزال */
            moveDeer();

            frame++;
        }

        drawForest();
        drawShelter();
        drawKids();

        if(night)
            drawDeer();

        drawPlayer();
        drawHUD();

        swiWaitForVBlank();

        if(gameOver)
        {
            clearScreen(C(1,2,3));
            rect(45,65,166,45,C(20,3,3));
            rect(55,75,146,25,C(3,3,3));

            while(1)
            {
                scanKeys();

                if(keysDown()&KEY_START)
                {
                    px=120; py=105;
                    mx=30; my=35;
                    wood=0;
                    rescued=0;
                    fireFuel=0;
                    shelter=0;
                    night=0;
                    day=1;
                    timer=0;
                    gameOver=0;

                    int i;
                    for(i=0;i<7;i++)
                        trees[i].alive=1;

                    for(i=0;i<3;i++)
                        kids[i].alive=1;

                    break;
                }

                swiWaitForVBlank();
            }
        }
    }

    return 0;
}
