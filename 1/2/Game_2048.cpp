/* 
详细伪代码（逐步说明）：
1. 目标：
   - 修复分数显示：把分数（SCORE 标签与数值）统一为深色文字（黑色或近黑色），并让数值在分数盒中垂直居中。
   - 避免第四行超出窗口：增大窗口或缩小面板。本实现选择增大窗口到 620x620，以保证现有格子尺寸（cell=100, gap=20）能完整显示。
   - 保持格子文字统一深色，去掉浅色文字阴影/叠加逻辑，仅在绘图相关函数（draw、swit、color）中调整布局与颜色。
2. 修改点：
   - 构造函数：将 initgraph 的尺寸从 550x550 改为 620x620。
   - draw():
     a) 调整分数盒子绘制参数：使用明确的左右上下坐标并计算内部垂直居中。
     b) 将分数标签与数值都设为深色，数值在盒中垂直与水平方向上（右侧）对齐并居中。
     c) 保留并微调面板和格子的尺寸、阴影、圆角等，但确保整体面板在新窗口中完全可见。
   - swit():
     a) 统一文字颜色为深色（RGB(67,62,58)），根据数字长度调整字号并水平垂直居中。
     b) 保持阴影、圆角、边框绘制，移除任何为浅色文字准备的特殊处理。
   - color(): 保留原色表逻辑，不改变。
3. 布局参数说明：
   - cell=100, gap=20, padding=10 -> tileAreaW = 460, boardW = 480。
   - boardX=40, boardY=100，boardY + boardW = 580 < windowHeight 620 -> 第四行可见。
4. 实现细节（伪代码）：
   - 在 draw() 中：
     - clear 背景
     - 绘制标题
     - 绘制分数盒：计算 boxTop, boxBottom, boxLeft, boxRight, boxHeight
     - 设置 label 字体，计算 label x = boxLeft + innerPadding，label y = boxTop + (boxHeight - labelHeight)/2
     - 设置 value 字体，计算 valueWidth、高度，并将 value x = boxRight - innerPadding - valueWidth，value y = boxTop + (boxHeight - valueHeight)/2
     - 绘制面板背景与阴影
     - 循环绘制 4x4 格子调用 swit()
     - 保持原有随机插入逻辑
   - 在 swit() 中：
     - 绘制阴影与圆角背景（使用 color(at)）
     - 根据数字字符串长度选择字号，计算 textwidth/textheight，居中绘制，文字颜色为深色
5. 仅修改绘图相关功能与窗口初始化，不改变游戏逻辑（移动、合并、随机等）。
*/

#define _CRT_SECURE_NO_WARNINGS
#include "Game_2048.h"
#include <cstdlib>
#include <ctime>
#include <graphics.h>
#include <algorithm>
#include <string>

/*
实现代码：调整窗口尺寸，分数文本居中，文字使用深色，保证第四行可见
*/

Game_2048::Game_2048()
{
	srand((unsigned int)time(NULL));
	// 增大窗口以确保 4 行格子在可视区域内（原 550x550 会导致底部超出）
	initgraph(620, 620, SHOWCONSOLE);
}

int Game_2048::start()
{
	init();
	while (true)
	{
		updata();
		if (isGameOver())
		{
			sprintf(ea, "%d", score);
			if (MessageBox(GetHWnd(), ea, "是否再来一局", MB_YESNO) == IDYES)
			{
				init();
				updata();
			}
			else
			{
				exit(0);
			}
		}
	}
	system("pause");
	return 0;
}

void Game_2048::init()
{
	score = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			arr[i][j] = 0;
		}
	}
	int x, y;
	//随机2个位置复制为初始位置
	for (int i = 0; i < 2; i++)
	{
		x = rand() % 4;
		y = rand() % 4;
		arr[x][y] = 2;
	}
}

// 根据实际数值选择颜色（使用 log2 映射）
void Game_2048::color(int value)
{
	// value 为实际数字（如 2,4,8...），如果为0使用空格颜色
	if (value == 0) {
		setfillcolor(RGB(205, 193, 180)); // 空格颜色（浅灰）
		return;
	}
	int idx = 0;
	int v = value;
	while (v > 1) { v >>= 1; idx++; } // 2 -> idx=1, 4->2 ...
	// 根据 idx 选择颜色（手动调配较和谐的颜色）
	switch (idx)
	{
	case 1: setfillcolor(RGB(238, 228, 218)); break; // 2
	case 2: setfillcolor(RGB(237, 224, 200)); break; // 4
	case 3: setfillcolor(RGB(242, 177, 121)); break; // 8
	case 4: setfillcolor(RGB(245, 149, 99));  break; // 16
	case 5: setfillcolor(RGB(246, 124, 95));  break; // 32
	case 6: setfillcolor(RGB(246, 94, 59));   break; // 64
	case 7: setfillcolor(RGB(237, 207, 114)); break; // 128
	case 8: setfillcolor(RGB(237, 204, 97));  break; // 256
	case 9: setfillcolor(RGB(237, 200, 80));  break; // 512
	default: setfillcolor(RGB(60, 58, 50));   break; // 大于 1024 使用深色
	}
}

int Game_2048::check()
{
	int a = 0;
	for (int i = 0; i < 4; i++) {

		for (int j = 0; j < 4; j++) {

			if (arr[i][j] == 0)
			{
				a++;
			}
		}
	}
	return a;
}

// 改善单个格子的绘制：带阴影、圆角、数字居中、数字统一使用深色以提高对比
void Game_2048::swit(int x, int y, int at)
{
	const int cell = 100;
	const int radius = 14; // 稍微小一点更协调
	// 阴影偏移与颜色
	int shadowOffset = 6;
	setfillcolor(RGB(150, 150, 150));
	roundrect(x + shadowOffset, y + shadowOffset, x + cell + shadowOffset, y + cell + shadowOffset, radius, radius);
	solidroundrect(x + shadowOffset, y + shadowOffset, x + cell + shadowOffset, y + cell + shadowOffset, radius, radius);

	// 绘制格子背景
	color(at);
	solidroundrect(x, y, x + cell, y + cell, radius, radius);

	// 绘制边框（使用更柔和的线）
	setlinecolor(RGB(230, 225, 220));
	setlinestyle(PS_SOLID, 1);
	roundrect(x, y, x + cell, y + cell, radius, radius);

	// 如果不是空格，绘制数字并居中
	if (at != 0)
	{
		char et[32] = { 0 };
		sprintf(et, "%d", at);

		// 根据数字长度调整字体大小
		int len = strlen(et);
		int fontSize = 48;
		if (len == 1) fontSize = 48;
		else if (len == 2) fontSize = 40;
		else if (len == 3) fontSize = 30;
		else fontSize = 22;

		settextstyle(fontSize, 0, "楷体");

		// 统一使用深色文字以确保任何浅色格子（如 16）也可读
		COLORREF darkText = RGB(67, 62, 58);

		int tw = textwidth(et);
		int th = textheight(et);
		int tx = x + (cell - tw) / 2;
		int ty = y + (cell - th) / 2;
		// 微调垂直以适配奇偶字体差异
		ty -= 2;

		settextcolor(darkText);
		outtextxy(tx, ty, et);
	}
}

int Game_2048::step(int x, int y, int mx, int my)
{
	int m = 0;
	int dx = x;
	int dy = y;

	while (true) {
		dx += mx;
		dy += my;

		if (!inArea(dx, dy))
		{
			break;
		}
		if (arr[dy][dx] == 0)
		{
			m++;
		}
	}
	return m;
}

int Game_2048::Move(int x, int y)
{
	int m = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (arr[j][i] != 0)
			{
				m = step(i, j, x, y);
				std::swap(arr[j + m * y][i + m * x], arr[j][i]);
				mergeNum(arr[j + m * y + y][i + m * x + x], arr[j + m * y][i + m * x]);
			}
		}
	}
	return 0;
}

void Game_2048::random()
{
	int x = 0, y = 0;
	while (true)
	{
		x = rand() % 4;
		y = rand() % 4;
		if (arr[x][y] == 0)
		{
			arr[x][y] = 2;
			break;
		}
	}
}

bool Game_2048::isGameOver()
{
	int end = 0;
	int dir[4][2] = { {1,0},{0,1},{-1,0},{0,-1} };
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (arr[i][j] != 0)
			{
				int flag = true;
				for (int z = 0; z < 4; z++)
				{
					int dx = dir[z][1] + arr[i][j];
					int dy = dir[z][0] + arr[i][j];
					if (!inArea(dx, dy))
						continue;
					if (arr[i][j] == arr[dy][dx])
					{
						flag = false;
						break;
					}
				}
				//4个方向都不等
				if (flag)
					end++;
			}
		}
	}
	return end == 16;
}

int Game_2048::keyDown()
{
	int fenshu = 0;
loop:
	char whpar = _getch();
	switch (whpar)
	{
	case 'w':
	case 'W':

		//fenshu += UP();
		Move(0, -1);
		break;
	case 'a':
	case 'A':
		Move(-1, 0);
		break;
	case 's':
	case 'S':
		Move(0, 1);
		break;
	case 'd':
	case 'D':
		Move(1, 0);
		break;
	default:
		goto loop;
		break;
	}
	return fenshu;
}

// 改善整体绘制：背景、标题、面板和格子，并确保分数垂直居中，字体为深色，第四行可见
void Game_2048::draw()
{
	BeginBatchDraw();

	// 背景
	setbkcolor(RGB(250, 248, 239));
	cleardevice();

	// 顶部标题与分数面板
	settextstyle(36, 0, "微软雅黑");
	settextcolor(RGB(119, 110, 101));
	outtextxy(30, 18, "2048");

	// 分数盒子（加宽以避免文字被裁或换行）
	int boxLeft = 340;
	int boxTop = 10;
	int boxRight = 540;
	int boxBottom = 64;
	int boxRadius = 12;
	int boxInnerPadding = 12;
	// 盒子背景
	setfillcolor(RGB(140, 128, 118)); // 稍深一点以增加对比
	solidroundrect(boxLeft, boxTop, boxRight, boxBottom, boxRadius, boxRadius);

	// 分数标签（深色以统一风格）
	const char* label = "SCORE";
	settextstyle(16, 0, "微软雅黑");
	COLORREF darkText = RGB(67, 62, 58);
	settextcolor(darkText);
	int labelW = textwidth(label);
	int labelH = textheight(label);
	int boxH = boxBottom - boxTop;
	int labelX = boxLeft + boxInnerPadding;
	int labelY = boxTop + (boxH - labelH) / 2;
	outtextxy(labelX, labelY, label);

	// 分数值（使用深色并垂直居中）
	sprintf(ea, "%d", score);
	settextstyle(20, 0, "楷体");
	settextcolor(darkText);
	int valW = textwidth(ea);
	int valH = textheight(ea);
	int valX = boxRight - boxInnerPadding - valW;
	int valY = boxTop + (boxH - valH) / 2;
	outtextxy(valX, valY, ea);

	// 游戏面板背景（居中并留出间距，避免重叠）
	const int cell = 100;
	const int gap = 20;
	const int padding = 10;
	int tileAreaW = 4 * cell + 3 * gap; // 460
	int boardX = 40;
	int boardY = 100;
	int boardW = tileAreaW + padding * 2; // 480

	// 面板阴影
	setfillcolor(RGB(160, 160, 160));
	solidroundrect(boardX + 8, boardY + 8, boardX + boardW + 8, boardY + boardW + 8, 26, 26);
	// 面板主体
	setfillcolor(RGB(187, 173, 160));
	solidroundrect(boardX, boardY, boardX + boardW, boardY + boardW, 26, 26);

	// 每个格子绘制：使用面板内偏移和间距，避免与标题/分数重叠
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			int x = boardX + padding + i * (cell + gap);
			int y = boardY + padding + j * (cell + gap);
			swit(x, y, arr[j][i]);
		}
	}

	// 如果有空位则随机一个新格子（保留原逻辑位置，注意这会在每次 draw 时触发，保持原行为）
	if (check() != 0)
	{
		random();
	}

	EndBatchDraw();
}

void Game_2048::text()
{
	// 保留旧的控制台输出行为（可选）
	//system("cls");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%d\t", arr[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void Game_2048::updata()
{
	draw();
	keyDown();
}

bool Game_2048::inArea(int x, int y)
{
	return !(x >= 4 || x < 0 || y >= 4 || y < 0);
}

void Game_2048::mergeNum(int& v1, int& v2)
{
	if (v1 == v2)
	{
		v1 *= 2;
		v2 = 0;
	}
}
