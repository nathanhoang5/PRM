#pragma once
#include <SDL/SDL.h>


enum class GameState{PLAY,EXIT};

class MainGame
{
public:
	MainGame();
	~MainGame();

	void run();

	


private:
	void initSystems();
	void gameLoop();
	void processInput();
	void drawGame();
	void createObstacle();
	void populate();
	bool notObstructed( int x1, int y1, int x2, int y2);
	void connect();
	int maxNum(int a, int b);
	int minNum(int a, int b);
	void redrawSF();
	void printCn();
	void query();
	int getMoveDist(int a, int b);
	void foundNode(int a);
	bool notFound(int a);
	void addNodes(int a);
	void clearQueueList();
	void redrawFin();
	void populateTestMap();

	SDL_Window* _window;
	SDL_Renderer* _renderer;
	int _screenWidth;
	int _screenHeight;
	GameState _gameState;
};

