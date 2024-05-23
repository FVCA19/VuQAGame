#include "Game.h"
#define TICK_INTERVAL 20
static Uint32 nextTime;
Uint32 TimeLeft() {
	Uint32 curTime;
	curTime = SDL_GetTicks();
	if (nextTime <= curTime) return 0;
	else					 return nextTime - curTime;
}

Game::Game() {
	window = SDL_CreateWindow( "Convey's Game of Life --- (paused)",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC );

	cout << "Game is running." << endl;
	isRunning = true;
}

Game::~Game() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	cout << "Endgame." << endl;
}

void Game::InitCells(int rows, int cols) {
	cells.clear();
	grid.rows = rows;
	grid.cols = cols;

	for (int x = 0; x < grid.rows; x++) {
		vector<Cell> column;
		for (int y = 0; y < grid.cols; y++) {
			column.push_back(Cell());
		}
		cells.push_back(column);
	}


}

void Game::Run() {
	InitCells(40, 40);
	AdjustGridSize();
	nextTime = SDL_GetTicks() + TICK_INTERVAL;

	while (isRunning) {

		SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
		SDL_RenderClear(renderer);

		Draw();
		SDL_RenderPresent(renderer);
		HandleEvents();

		if (isRunningSim && !mouseDown)
			RunSimulation();

		SDL_Delay(TimeLeft());
		nextTime += TICK_INTERVAL;
	}


	cout << "Game loop ended." << endl;
}

void Game::HandleEvents() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {

		if (ev.type == SDL_MOUSEMOTION) {
			SDL_GetMouseState(&mouseX, &mouseY);
		}

		if (ev.type == SDL_MOUSEBUTTONDOWN) {
			if (ev.button.button == SDL_BUTTON_LEFT)
				mouseDown = true;
		}

		if (ev.type == SDL_MOUSEBUTTONUP) {
			if (ev.button.button == SDL_BUTTON_LEFT)
				if (mouseDown)
					mouseDown = false;
		}

		if (ev.type == SDL_WINDOWEVENT) {

			if (ev.window.event == SDL_WINDOWEVENT_CLOSE)
				isRunning = false;

			if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
				AdjustGridSize();

			if (ev.window.event == SDL_WINDOWEVENT_RESTORED)
				AdjustGridSize();
		}

		if (ev.type == SDL_KEYDOWN) {

			if (ev.key.keysym.sym == SDLK_ESCAPE)
				isRunning = false;

			if (ev.key.keysym.sym == SDLK_SPACE) {
				isRunningSim = !isRunningSim;
				if (isRunningSim)		SDL_SetWindowTitle(window, "Convey's Game of Life --- (running)");
				else if (!isRunningSim)	SDL_SetWindowTitle(window, "Convey's Game of Life --- (paused)");
			}
		}
	}

	if (mouseDown) cells[mouseCellX][mouseCellY].Flip();
}

void Game::Draw() {
    for (int x = 0; x < grid.rows; x++) {
        for (int y = 0; y < grid.cols; y++) {
            SDL_FRect cellRect = {
                1.0f + (float)grid.posX + ((float)x * grid.cellWidth),
                1.0f + (float)grid.posY + ((float)y * grid.cellHeight),
                grid.cellWidth,
                grid.cellHeight
            };
            if (cells[x][y].isAlive)
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            else
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRectF(renderer, &cellRect);

            if (mouseX > cellRect.x && mouseX < cellRect.x + cellRect.w) {
                if (mouseY > cellRect.y && mouseY < cellRect.y + cellRect.h) {
                    mouseCellX = x;
                    mouseCellY = y;
                    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 15);
                    SDL_RenderFillRectF(renderer, &cellRect);
                }
            }
        }
    }


    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (int i = 0; i <= grid.rows; i++) {
        SDL_FRect vLine = {
            grid.posX + (i * grid.cellWidth) + 1, grid.posY + 1, 2,
            (i == grid.rows) ? grid.height + 2 : grid.height
        };
        SDL_RenderFillRectF(renderer, &vLine);
    }
    for (int i = 0; i <= grid.cols; i++) {
        SDL_FRect hLine = {
            grid.posX + 1,
            grid.posY + (i * grid.cellHeight) + 1, (i == grid.cols) ? grid.width + 2 : grid.width, 2
        };
        SDL_RenderFillRectF(renderer, &hLine);
    }
}

void Game::AdjustGridSize() {
	int ww = 0;
	int hh = 0;
	SDL_GetWindowSize(window, &ww, &hh);

	int maxLength = 0;
	if (ww > hh) maxLength = hh;
	else		 maxLength = ww;
	grid.posX			= grid.borderSize;
	grid.posY			= grid.borderSize;
	int botRightX		= maxLength - grid.borderSize;
	int botRightY		= maxLength - grid.borderSize;
	grid.width		    = botRightX - grid.posX;
	grid.height			= botRightY - grid.posY;
	grid.cellWidth		= (float)grid.width / (float)grid.rows;
	grid.cellHeight		= (float)grid.height / (float)grid.cols;
	if (ww > hh) grid.posX = ((float)ww / 2.0f) - ((float)grid.width / 2.0f);
	else		 grid.posY = ((float)hh / 2.0f) - ((float)grid.height / 2.0f);
}
int Game::iX(int i) {
	if (i < 0)					    return 0;
	else if (i > grid.rows - 1)		return grid.rows - 1;
	else							return i;
}
int Game::iY(int i) {
	if (i < 0)					    return 0;
	else if (i > grid.cols - 1)		return grid.cols - 1;
	else							return i;
}
void Game::RunSimulation() {
	for (int x = 0; x < cells.size(); x++) {
		for (int y = 0; y < cells[x].size(); y++) {

			// Reset the neighbour count
			cells[x][y].neighbours = 0;
			// Top left
			if (cells[iX(x - 1)][iY(y - 1)].isAlive)	cells[x][y].neighbours++;
			// Top
			if (cells[iX(x)][iY(y - 1)].isAlive)		cells[x][y].neighbours++;
			// Top right
			if (cells[iX(x + 1)][iY(y - 1)].isAlive)	cells[x][y].neighbours++;
			// Left
			if (cells[iX(x - 1)][iY(y)].isAlive)		cells[x][y].neighbours++;
			// Right
			if (cells[iX(x + 1)][iY(y)].isAlive)		cells[x][y].neighbours++;
			// Bottom Left
			if (cells[iX(x - 1)][iY(y + 1)].isAlive)	cells[x][y].neighbours++;
			// Bottom
			if (cells[iX(x)][iY(y + 1)].isAlive)		cells[x][y].neighbours++;
			// Bottom right
			if (cells[iX(x + 1)][iY(y + 1)].isAlive)	cells[x][y].neighbours++;
		}
	}

	while (simTick < simDelay) {
		simTick++;
		SDL_Delay(1);
	}

	for (int x = 0; x < cells.size(); x++) {
		for (int y = 0; y < cells[x].size(); y++) {
			if (cells[x][y].neighbours == 3)	cells[x][y].Born();
			if (cells[x][y].neighbours < 2)		cells[x][y].Die();
			if (cells[x][y].neighbours > 3)		cells[x][y].Die();
		}
	}
	simTick = 0;
}
