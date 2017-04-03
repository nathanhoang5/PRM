#include "MainGame.h"
#include <iostream>
#include <string>
#include <iostream>
#include <iomanip>
#include <queue>
#include <string>
#include <math.h>
#include <ctime>
#include <stdio.h>     
#include <stdlib.h> 

using namespace std;

//populate
const int numNodes = 2000;
int startX = 120;
int startY = 130;
int endX = 400;
int endY = 130;
const int nodeSize = 3;
bool stillRunning = false;
bool selectPts = true;

//create obstacles
const int numObs = 3;
SDL_Rect* obs = new SDL_Rect[numObs];

//connect
const int maxNodeDist = 50;

//query
string pathList;
int * closedNodeList = new int [numNodes];
int * addedNodes = new int[numNodes];
int closedCounter = 0;
int aNCounter = 0;

class node
{

	// current position
	int xPos;
	int yPos;
	// total distance already travelled to reach the node
	int level = 0;
	// priority=level+remaining distance estimate
	int priority;  // smaller: higher priority
	int parent;
	int arrayValue;
	int * connections = new int [numNodes];
	int connectionCounter = 0;

public:
	node(int xp, int yp, int d, int p, int a)
	{
		xPos = xp; yPos = yp; level = d; priority = p; arrayValue = a;
		//cout << arrayValue << endl;
	}

	int getxPos() const { return xPos; }
	int getyPos() const { return yPos; }
	int getLevel() const { return level; }
	int getPriority() const { return priority; }
	int getParent() const { return parent; }
	int getArrayValue() const { 
		//cout << arrayValue << endl;
		return arrayValue; 
	}
	int getConnection(int n) {	return connections[n];	}

	void setPriority(int pD)
	{
		mvCost(pD);
		priority = level;
	}

	// give better priority to going strait instead of diagonally
	void mvCost(int i) // i: direction
	{
		level = i;
	}

	// Estimation function for the remaining distance to the goal.
	const int & estimate() const
	{
		static int xd, yd, d;
		xd = endX - xPos;
		yd = endY - yPos;
		d = static_cast<int>(sqrt(xd*xd + yd*yd));
		return(d);
	}

	void setParent(int par) {
		parent = par;
	}

	void initCArray() {
		for (int i = 0; i < numNodes; i++) {
			connections[i] = -1;
			//cout << connections[i] << endl;
		}
	}


	void addConnection(int c) {
		connections[connectionCounter] = c;		
		//cout << "Node " << arrayValue << " connected to node " << connections[connectionCounter] << ". CC is now " << connectionCounter << endl;
		connectionCounter = connectionCounter + 1;

		//cout << "Node " << arrayValue << " connected to node " << c << ". CC is now " << connectionCounter << endl;
	}

	void printConnections() {
		for (int i = 0; i < numNodes; i++) {
			if (connections[i] == -1) break;
			else {
				//cout << connections[i];
			}
		}
	}
	
};

class CompareNode {
public:
	bool operator()(node & n1, node & n2)
	{
		
		return n1.getPriority() > n2.getPriority();
		
	}
};

node* nodeList[numNodes];

MainGame::MainGame()
{
	_window = nullptr;
	_renderer = nullptr;
	_screenWidth = 512;
	_screenHeight = 288;
	_gameState = GameState::PLAY;
}

void fatalError(string errorString) {
	cout << errorString << endl;
	cout << "Enter any key to quit...";
	int tmp;
	cin >> tmp;
	SDL_Quit();
	exit(1);
}

MainGame::~MainGame()
{

}


void MainGame::run() {
	initSystems();
	createObstacle();
	if (selectPts) {
		cout << "Select start point" << endl;
	}
	else {
		stillRunning = true;
		redrawSF();
		cout << "Press space to populate map" << endl;
	}
	
	gameLoop();	
}

void MainGame::initSystems() {
	
	SDL_Init(SDL_INIT_EVERYTHING);
	_window = SDL_CreateWindow("Probabilistic Roadmap", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, SDL_WINDOW_SHOWN);
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	if (_window == nullptr) {
		fatalError("SDL Window could not be created!");
	}
	SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
	SDL_RenderClear(_renderer);

	for (int i = 0; i < numNodes; i++) {
		closedNodeList[i] = -1;
		addedNodes[i] = -1;
		//cout << connections[i] << endl;
	}
	closedNodeList[0] = 0;
	closedCounter = closedCounter + 1;
}


void MainGame::gameLoop() {
	while (_gameState != GameState::EXIT) {
		processInput();
		//drawGame();
	}
}
int counter = 0;
clock_t start, i1, i2, i3, i4, endClock;
bool selectStart = true;
void MainGame::processInput() {
	SDL_Event evnt;

	while (SDL_PollEvent(&evnt) == true) {
		switch (evnt.type) {
			case SDL_QUIT:
				_gameState = GameState::EXIT;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (selectPts&&selectStart) {
					SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255); 
					SDL_RenderClear(_renderer);
					createObstacle();
					startX = evnt.button.x;
					startY = evnt.button.y;
					selectStart = false;
					cout << "Select end point" << endl;
				}
				else if (selectPts&&selectStart == false) {
					endX = evnt.button.x;
					endY = evnt.button.y;
					selectStart = true;
					selectPts = false;
					stillRunning = true;
					redrawSF();
					cout << "Press space to populate map" << endl;
				}
			//case SDL_MOUSEMOTION:
			//	cout << evnt.motion.x << " " << evnt.motion.y << endl;
			case SDL_KEYDOWN:
				if (stillRunning) {
					if (evnt.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						if (counter == 0) {
							start = clock();
							populate();
							//populateTestMap();
							createObstacle();
							i1 = clock();
							cout << "Press space to connect nodes" << endl;
							counter = counter + 1;

						}

						else if (counter == 1) {
							i2 = clock();
							connect();
							i3 = clock();
							cout << "Press space to find path (A*)" << endl;
							counter = counter + 1;
						}
						else if (counter == 2) {
							counter = 0;
							i4 = clock();
							query();
							endClock = clock();
							double time_elapsed = double(i1 - start) + double(i3 - i2) + double(endClock - i4);
							cout << "Time to calculate the route (ms): " << time_elapsed << endl;
			
							stillRunning = false;
							selectPts = false;
							break;
						}
					}
				}
		}
	}
}


//Populates a 1024x576 map of numNodes nodes
void MainGame::populate() {
	
	
	static node* a;
	
	
	//Starting point
	nodeList[0] = new node(startX, startY, 0, 10000, 0);
	nodeList[0]->initCArray();
	nodeList[0]->setParent(-5);
	SDL_Rect startRect;
	startRect.h = nodeSize;
	startRect.w = nodeSize;
	startRect.x = startX-nodeSize/2;
	startRect.y = startY-nodeSize/2;
	SDL_SetRenderDrawColor(_renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(_renderer, &startRect);
	
	//Finish point
	nodeList[1] = new node(endX, endY, 0, 10000, 1);
	nodeList[1]->initCArray();
	SDL_Rect finRect;
	finRect.h = nodeSize;
	finRect.w = nodeSize;
	finRect.x = endX-nodeSize/2;
	finRect.y = endY-nodeSize/2;
	SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(_renderer, &finRect);
	
	SDL_SetRenderDrawColor(_renderer, 0, 0, 255, 255);

	for (int i = 2; i < numNodes; i++) {
		int randX = rand() % _screenWidth;
		int randY = rand() % _screenHeight;
		
		a = new node(randX, randY, 0, 10000, i);
		
		nodeList[i] = a;
		nodeList[i]->initCArray();
		SDL_Rect nodeRect;
		nodeRect.h = nodeSize;
		nodeRect.w = nodeSize;
		nodeRect.x = randX-nodeSize/2;
		nodeRect.y = randY-nodeSize/2;
		SDL_RenderFillRect(_renderer, &nodeRect);
		//cout << "Node " << i << " at point (" << randX << "," << randY << ")" << endl;
	}


	SDL_RenderPresent(_renderer);

	

	cout << "Populated nodes!" << endl;
} 

void MainGame::populateTestMap() {
	static node* a;


	//Starting point
	nodeList[0] = new node(startX, startY, 0, 10000, 0);
	nodeList[0]->initCArray();
	nodeList[0]->setParent(-5);
	SDL_Rect startRect;
	startRect.h = nodeSize;
	startRect.w = nodeSize;
	startRect.x = startX - nodeSize / 2;
	startRect.y = startY - nodeSize / 2;
	SDL_SetRenderDrawColor(_renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(_renderer, &startRect);

	//Finish point
	nodeList[1] = new node(endX, endY, 0, 10000, 1);
	nodeList[1]->initCArray();
	SDL_Rect finRect;
	finRect.h = nodeSize;
	finRect.w = nodeSize;
	finRect.x = endX - nodeSize / 2;
	finRect.y = endY - nodeSize / 2;
	SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(_renderer, &finRect);

	SDL_SetRenderDrawColor(_renderer, 0, 0, 255, 255);

	nodeList[2] = new node(378, 86, 0, 10000, 2);
	nodeList[3] = new node(388,47, 0, 10000, 3);
	nodeList[4] = new node(377, 30, 0, 10000, 4);
	nodeList[5] = new node(336, 14, 0, 10000, 5);
	nodeList[6] = new node(324, 32, 0, 10000, 6);
	nodeList[7] = new node(285, 10, 0, 10000, 7);
	nodeList[8] = new node(273, 8, 0, 10000, 8);
	nodeList[9] = new node(225, 11, 0, 10000, 9);
	nodeList[10] = new node(190, 4, 0, 10000, 10);
	nodeList[11] = new node(153, 21, 0, 10000, 11);
	nodeList[12] = new node(105, 15, 0, 10000, 12);
	nodeList[13] = new node(94, 46, 0, 10000, 13);
	nodeList[14] = new node(90, 93, 0, 10000, 14);
	nodeList[15] = new node(107, 100, 0, 10000, 15);
	nodeList[16] = new node(120, 65, 0, 10000, 16);
	nodeList[17] = new node(112, 25, 0, 10000, 17);
	
	for (int i = 2; i < numNodes; i++) {
		nodeList[i]->initCArray();
		SDL_Rect nodeRect;
		nodeRect.h = nodeSize;
		nodeRect.w = nodeSize;
		nodeRect.x = nodeList[i]->getxPos() - nodeSize / 2;
		nodeRect.y = nodeList[i]->getyPos() - nodeSize / 2;
		SDL_RenderFillRect(_renderer, &nodeRect);
	
	}

}

void MainGame::createObstacle() {
	
	obs[0].h = 180;
	obs[0].w = 40;
	obs[0].x = 160;
	obs[0].y = 50;

	obs[1].h = 180;
	obs[1].w = 40;
	obs[1].x = 330;
	obs[1].y = 50;

	obs[2].h = 40;
	obs[2].w = 130;
	obs[2].x = 200;
	obs[2].y = 120;
	

	SDL_SetRenderDrawColor(_renderer, 75, 0, 130, 255);
	for (int i = 0; i < numObs; i++) {
		SDL_RenderFillRect(_renderer, &obs[i]);
	}
	SDL_RenderPresent(_renderer);
}

void MainGame::connect() {

	
	int connectionsCounter = 0;
	SDL_SetRenderDrawColor(_renderer, 0, 0, 255, 255);
	//current node
	for (int i = 0; i < numNodes; i++) {
		
		//check all nodes for connections
		//nodeList[i]->initCArray();
		for (int j = 0; j < numNodes; j++){
			if (i == j) {}
			else {
				int dX = nodeList[i]->getxPos() - nodeList[j]->getxPos();
				int dY = nodeList[i]->getyPos() - nodeList[j]->getyPos();
				if (static_cast<int>(sqrt(dX*dX + dY*dY)) < maxNodeDist 
					&& notObstructed(nodeList[i]->getxPos(), nodeList[i]->getyPos(), nodeList[j]->getxPos(), nodeList[j]->getyPos())) {
					nodeList[i]->addConnection(j);
					//cout << "Node " << i << " connected to node " << j << endl;
					SDL_RenderDrawLine(_renderer, nodeList[i]->getxPos(), nodeList[i]->getyPos(), nodeList[j]->getxPos(), nodeList[j]->getyPos());
				}
			}
		}
	}
	SDL_RenderPresent(_renderer);
	cout << "Connected Nodes!" << endl;
	createObstacle();
	redrawSF();
	
}
	
	

bool MainGame::notObstructed(int x1, int y1, int x2, int y2) {
	for (int i = 0; i < numObs; i++) {
		if ((y1 - y2) == 0 && (
			(obs[i].x <maxNum(x1,x2) && obs[i].x>minNum(x1,x2)&&y1>obs[i].y && y1<obs[i].y+obs[i].h) 
			||(obs[i].x+obs[i].w<maxNum(x1,x2) && obs[i].x + obs[i].w>minNum(x1, x2) && y1>obs[i].y && y1<obs[i].y + obs[i].h)) ){
			return false;
		}
		else if ((x1 - x2) == 0 && (
			(obs[i].y <maxNum(y1, y2) && obs[i].y>minNum(y1, y2) && x1>obs[i].x && x1<obs[i].x + obs[i].w)
			|| (obs[i].y +obs[i].h<maxNum(y1, y2) && obs[i].y+obs[i].h>minNum(y1, y2) && x1>obs[i].x && x1<obs[i].x + obs[i].w)
			)) {
			return false;
		}
		else {
			float dX = (float)(x2 - x1);
			float dY = (float)(y2 - y1);
			float slope = dY / dX;
			float a = (float)(obs[i].x);
			float b = (float)(obs[i].x+obs[i].w);
			float c = (float)(obs[i].y);
			float d = (float)(obs[i].y + obs[i].h);

			if ( ((((slope*(a-x1)+y1) > obs[i].y) && ((slope*(a - x1) + y1) < obs[i].y+obs[i].h)) && (a<=maxNum(x1,x2) && a>=minNum(x1,x2)))
				|| ((((slope*(b - x1) + y1) > obs[i].y) && ((slope*(b - x1) + y1) < obs[i].y + obs[i].h)) && (b <= maxNum(x1, x2) && b >= minNum(x1, x2))) 
				|| (((((c - y1)/slope + x1) > obs[i].x) && ((c - y1)/slope + x1) < obs[i].x + obs[i].w) && (c <= maxNum(y1, y2) && c >= minNum(y1, y2))) 
				|| (((((d - y1) / slope + x1) > obs[i].x) && ((d - y1) / slope + x1) < obs[i].x + obs[i].w) && (d <= maxNum(y1, y2) && d >= minNum(y1, y2)))) {
				
				return false;
			}

		}
	}
	return true;
}

int MainGame::maxNum(int a, int b) {
	if (a > b)
		return a;
	else
		return b;
}

int MainGame::minNum(int a, int b) {
	if (a < b)
		return a;
	else
		return b;
}

void MainGame::redrawSF() {
	//Starting point
	SDL_Rect startRect;
	startRect.h = nodeSize;
	startRect.w = nodeSize;
	startRect.x = startX-nodeSize/2;
	startRect.y = startY - nodeSize / 2;
	SDL_SetRenderDrawColor(_renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(_renderer, &startRect);

	//Finish point
	SDL_Rect finRect;
	finRect.h = nodeSize;
	finRect.w = nodeSize;
	finRect.x = endX - nodeSize / 2;
	finRect.y = endY - nodeSize / 2;
	SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(_renderer, &finRect);

	SDL_RenderPresent(_renderer);
}

void MainGame::printCn() {
	for (int i = 0; i < numNodes; i++) {
		for (int j = 0; j < numNodes; j++) {
			if (!(nodeList[i]->getConnection(j)==-1)) {
				cout << "Node " << i << " connected to node " << nodeList[i]->getConnection(j) << endl;
			}
			else {
				break;
			}
		}
	}
}

priority_queue<node, vector<node>, CompareNode> pq;

void MainGame::query() {
	bool found = false;
	
	pq.push(*nodeList[0]);
	//cout << pq.top().getArrayValue() << endl;
	//cout << closedNodeList[0] << endl;
	while (found == false) {

		if (pq.size() == 0) {
			cout<<"No path to end!"<<endl;
			gameLoop();

		
		}
		int i = pq.top().getArrayValue();
		//cout << pq.top().getArrayValue() << endl;
			for (int j = 0; j < numNodes; j++) {
				int cxn = nodeList[i]->getConnection(j);
				if (nodeList[i]->getParent() == cxn) {
				}
				else if (cxn == 1) {
					cout << "Found path!" << endl;
					foundNode(i);
					found = true;
					break;
				}
				else if ((!(cxn == -1))&&notFound(cxn)) {
					/*nodeList[cxn]->setParent(i);
					nodeList[cxn]->setPriority(getMoveDist(i, cxn));
					pq.push(*nodeList[cxn]);
					closedNodeList[closedCounter] = cxn;
					closedCounter = closedCounter + 1;
					cout << "Created node " << cxn << endl;
					*/
					addedNodes[aNCounter] = cxn;
					aNCounter = aNCounter + 1;

				}
				else if ((!(cxn == -1)) && (notFound(cxn)==false)) {
					
					if(getMoveDist(i, cxn)<nodeList[cxn]->getLevel()){
						//cout << "yes" << endl;
						nodeList[cxn]->setParent(i);
						nodeList[cxn]->setPriority(getMoveDist(i, cxn));
					}

				}
 				else if(cxn == -1){
					//cout << "Cleared node!" << endl;
					pq.pop();
					addNodes(i);
					break;
				}
			}

	
	}
	//cout << "Queried!" << endl;
}

int MainGame::getMoveDist(int a, int b) {
	int dX = nodeList[a]->getxPos() - nodeList[b]->getxPos();
	int dY = nodeList[a]->getyPos() - nodeList[b]->getyPos();
	int d =(sqrt(dX*dX + dY*dY));
	return nodeList[a]->getLevel()+d;

}

void MainGame::foundNode(int a) {
 
	redrawFin();
	int curNode = a;
	SDL_SetRenderDrawColor(_renderer, 22, 204, 28, 255);
	pathList += to_string(nodeList[curNode]->getArrayValue());
	pathList += " ";
	SDL_RenderDrawLine(_renderer, nodeList[curNode]->getxPos(), nodeList[curNode]->getyPos(), endX, endY);
	while (!(nodeList[curNode]->getParent() == -5)) {
		
		pathList += to_string(nodeList[curNode]->getParent());
		pathList += " ";
		SDL_RenderDrawLine(_renderer, nodeList[curNode]->getxPos(), nodeList[curNode]->getyPos(), nodeList[nodeList[curNode]->getParent()]->getxPos(), nodeList[nodeList[curNode]->getParent()]->getyPos());
		
		curNode = nodeList[curNode]->getParent();
	}
	cout << "Path: " << pathList << endl;
	SDL_RenderPresent(_renderer);

}

bool MainGame::notFound(int a) {
	for (int i = 0; i < numNodes; i++) {
		if (closedNodeList[i] == a) {
			return false;
		}
		if (closedNodeList[i] == -1) {
			return true;
		}
	}

	return true;

}

void MainGame::addNodes(int a) {
	for (int i = 0; i < numNodes; i++) {
		if (addedNodes[i] == -1) {
			clearQueueList();
			break;
		}
		else {
			nodeList[addedNodes[i]]->setParent(a);
			nodeList[addedNodes[i]]->setPriority(getMoveDist(a, addedNodes[i]));
			pq.push(*nodeList[addedNodes[i]]);
			closedNodeList[closedCounter] = addedNodes[i];
			closedCounter = closedCounter + 1;
			//cout << "Created node " << addedNodes[i] << endl;
				
		}
	}
}

void MainGame::clearQueueList() {
	aNCounter = 0;
	for (int i = 0; i < numNodes; i++) {
		addedNodes[i] = -1;
		//cout << connections[i] << endl;
	}
}


void MainGame::redrawFin() {
	SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
	SDL_RenderClear(_renderer);
	SDL_SetRenderDrawColor(_renderer, 0, 0, 255, 255);
	for (int i = 0; i < numNodes; i++) {
		SDL_Rect nodeRect;
		nodeRect.h = nodeSize;
		nodeRect.w = nodeSize;
		nodeRect.x = nodeList[i]->getxPos() - nodeSize / 2;
		nodeRect.y = nodeList[i]->getyPos() - nodeSize / 2;
		SDL_RenderFillRect(_renderer, &nodeRect);
	}
	redrawSF();
	createObstacle();
}