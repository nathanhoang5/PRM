#include "MainGame.h"
#include <iostream>
#include <iomanip>
#include <queue>
#include <string>
#include <math.h>
#include <ctime>
#include <stdio.h>     
#include <stdlib.h> 

using namespace std;

//Number of nodes
const int numNodes = 2000;

//Start and end positions
int startX = 120;
int startY = 130;
int endX = 400;
int endY = 130;

//Size of node rectangles
const int nodeSize = 3;

//True after start and end have been selected, takes spacebar input
bool stillRunning = false;

//True at start, allows selection of start and end points
bool selectPts = true;

//Number of obstacles
const int numObs = 3;

//Array storing all obstacles
SDL_Rect* obs = new SDL_Rect[numObs];

//Max distance allowed between nodes
const int maxNodeDist = 50;

//Stores path from start to finish
string pathList;

//List of closed nodes
int * closedNodeList = new int [numNodes];

//List of new connections to be added, cleared for each new node evaluated
int * addedNodes = new int[numNodes];

//Counter for index of closed node array
int closedCounter = 0;

//Counter for index of added nodes array
int aNCounter = 0;

//Each point stored as a node
class node
{
	//Current position
	int xPos;
	int yPos;
	//Total distance already travelled to reach the node
	int level = 0;
	//Priority=distance from start node
	int priority;  // smaller: higher priority
	//Parent node
	int parent;
	//Value of node in node array
	int arrayValue;
	//List of node connections
	int * connections = new int [numNodes];
	//Counter for index of added nodes array
	int connectionCounter = 0;

public:
	node(int xp, int yp, int d, int p, int a)
	{
		xPos = xp; yPos = yp; level = d; priority = p; arrayValue = a;
	}

	int getxPos() const { return xPos; }
	int getyPos() const { return yPos; }
	int getLevel() const { return level; }
	int getPriority() const { return priority; }
	int getParent() const { return parent; }
	int getArrayValue() const { return arrayValue; }
	int getConnection(int n) {	return connections[n];	}

	//Sets movement cost to get to this node, then the priority (for the priority queue)
	void setPriority(int pD)
	{
		level = pD;
		priority = level+estimate();
	}

	// Estimation function for the remaining distance to the goal (can be used in priority for A*)
	const int & estimate() const
	{
		static int xd, yd, d;
		xd = endX - xPos;
		yd = endY - yPos;
		d = static_cast<int>(sqrt(xd*xd + yd*yd));
		return(d);
	}

	//Sets the parent of node
	void setParent(int par) {
		parent = par;
	}

	//Initializes all values of connection array
	void initCArray() {
		for (int i = 0; i < numNodes; i++) {
			connections[i] = -1;
		}
	}

	//Adds connection to a neighboring node
	void addConnection(int c) {
		connections[connectionCounter] = c;		
		connectionCounter = connectionCounter + 1;
	}
	
	//Prints all connections
	void printConnections() {
		for (int i = 0; i < numNodes; i++) {
			if (connections[i] == -1) break;
			else {
				cout << connections[i];
			}
		}
	}
	
};

//Used to determine order of priority queue (lower priority/move distance is higher)
class CompareNode {
public:
	bool operator()(node & n1, node & n2)
	{	
		return n1.getPriority() > n2.getPriority();
	}
};

//List of nodes
node* nodeList[numNodes];

//Initialize window parameters
MainGame::MainGame()
{
	_window = nullptr;
	_renderer = nullptr;
	_screenWidth = 512;
	_screenHeight = 288;
	_gameState = GameState::PLAY;
}

//Can be called to exit application when error is thrown
void fatalError(string errorString) {
	cout << errorString << endl;
	cout << "Enter any key to quit...";
	int tmp;
	cin >> tmp;
	SDL_Quit();
	exit(1);
}

//Destructor?? I don't know what this is...
MainGame::~MainGame()
{

}

//Called from main class, starts application depending on if selectPoints==true, then starts gameLoop()
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

//Create window and initialize lists, makes white background
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
	}
	closedNodeList[0] = 0;
	closedCounter = closedCounter + 1;
}

//If there is no error, continue to process input
void MainGame::gameLoop() {
	while (_gameState != GameState::EXIT) {
		processInput();
		//drawGame();
	}
}

//Keeps track of stage (populate, connect, query)
int counter = 0;
//Records time to execute algorithm
clock_t start, i1, i2, i3, i4, endClock;
//True if selecting start position, false if selecting end position
bool selectStart = true;
//Takes user input
void MainGame::processInput() {
	SDL_Event evnt;
	while (SDL_PollEvent(&evnt) == true) {
		switch (evnt.type) {
		//If exit is clicked, close application	
		case SDL_QUIT:
				_gameState = GameState::EXIT;
				break;
			//Mouse is clicked
			case SDL_MOUSEBUTTONDOWN:
				//Select start position
				if (selectPts&&selectStart) {
					SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255); 
					SDL_RenderClear(_renderer);
					createObstacle();
					startX = evnt.button.x;
					startY = evnt.button.y;
					selectStart = false;
					cout << "Select end point" << endl;
				}
				//Select end position
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
			
			//If spacebar pressed
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
							//Adds time taken for each section and print
							double time_elapsed = double(i1 - start) + double(i3 - i2) + double(endClock - i4);
							cout << "Time to calculate the route (ms): " << time_elapsed << endl;
							//End program
							stillRunning = false;
							selectPts = false;
							break;
						}
					}
				}
		}
	}
}


//Populates map of numNodes nodes
void MainGame::populate() {
	
	//Used to populate array
	static node* a;
	
	//Starting point
	nodeList[0] = new node(startX, startY, 0, 10000, 0);
	nodeList[0]->initCArray();
	nodeList[0]->setParent(-5); //Parent is -5
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
	
	//Blue
	SDL_SetRenderDrawColor(_renderer, 0, 0, 255, 255);

	//Fill the rest of the array with random nodes
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
	}

	SDL_RenderPresent(_renderer);
	cout << "Populated nodes!" << endl;
} 

//Populates a map with chosen points
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

//Sets position and draws obstacles. Can be called again to redraw
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

//Connects nodes to its neighbours
void MainGame::connect() {

	//Number of connections
	int connectionsCounter = 0;
	SDL_SetRenderDrawColor(_renderer, 0, 0, 255, 255);
	//i = current node
	for (int i = 0; i < numNodes; i++) {
		//Check all nodes for possible connections (j=other nodes)
		for (int j = 0; j < numNodes; j++){
			//Don't connect a node to itself
			if (i == j) {}
			//Two different nodes:
			else {
				int dX = nodeList[i]->getxPos() - nodeList[j]->getxPos();
				int dY = nodeList[i]->getyPos() - nodeList[j]->getyPos();
				//If nodes are less than the max distance apart and are not blocked by an obstacle, connect and draw line
				if (static_cast<int>(sqrt(dX*dX + dY*dY)) < maxNodeDist 
					&& notObstructed(nodeList[i]->getxPos(), nodeList[i]->getyPos(), nodeList[j]->getxPos(), nodeList[j]->getyPos())) {
					nodeList[i]->addConnection(j);
					SDL_RenderDrawLine(_renderer, nodeList[i]->getxPos(), nodeList[i]->getyPos(), nodeList[j]->getxPos(), nodeList[j]->getyPos());
				}
			}
		}
	}
	SDL_RenderPresent(_renderer);
	cout << "Connected Nodes!" << endl;
	//Re-draw obstacles and start/finish
	createObstacle();
	redrawSF();
}
	
	
//Returns true if two nodes are not obstructed
bool MainGame::notObstructed(int x1, int y1, int x2, int y2) {
	//Check each obstacle
	for (int i = 0; i < numObs; i++) {
		//Horizontal line check
		if ((y1 - y2) == 0 && (
			(obs[i].x <maxNum(x1,x2) && obs[i].x>minNum(x1,x2)&&y1>obs[i].y && y1<obs[i].y+obs[i].h) 
			||(obs[i].x+obs[i].w<maxNum(x1,x2) && obs[i].x + obs[i].w>minNum(x1, x2) && y1>obs[i].y && y1<obs[i].y + obs[i].h)) ){
			return false;
		}
		//Vertical line check
		else if ((x1 - x2) == 0 && (
			(obs[i].y <maxNum(y1, y2) && obs[i].y>minNum(y1, y2) && x1>obs[i].x && x1<obs[i].x + obs[i].w)
			|| (obs[i].y +obs[i].h<maxNum(y1, y2) && obs[i].y+obs[i].h>minNum(y1, y2) && x1>obs[i].x && x1<obs[i].x + obs[i].w)
			)) {
			return false;
		}
		//Regular line check
		else {
			//Check each equation for each edge of rectangle
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

//Returns max of two integers
int MainGame::maxNum(int a, int b) {
	if (a > b)
		return a;
	else
		return b;
}

//Returns min of two integers
int MainGame::minNum(int a, int b) {
	if (a < b)
		return a;
	else
		return b;
}

//Re-draw start and finish rectangles for clarity
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

//Print all connections for all nodes
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

//Priority queue to store open (activated) nodes
priority_queue<node, vector<node>, CompareNode> pq;

//Finds the best path!
void MainGame::query() {
	//True when path is found
	bool found = false;
	//Add starting node to pq
	pq.push(*nodeList[0]);
	//While the path hasn't been found...
	while (found == false) {
		//If there are no more open nodes, there is no possible path
		if (pq.size() == 0) {
			cout<<"No path to end!"<<endl;
			gameLoop();
		}
		//The current node being evaluated
		int i = pq.top().getArrayValue();
			//j is index in the current node's connection list
			for (int j = 0; j < numNodes; j++) {
				//One of current node's connections, will be -1 if node has no more connections
				int cxn = nodeList[i]->getConnection(j);
				//Do nothing if the connection is its parent
				if (nodeList[i]->getParent() == cxn) {
				}
				//If the connection is the finish, break out of loop and display path
				else if (cxn == 1) {
					cout << "Found path!" << endl;
					foundNode(i);
					found = true;
					break;
				}
				//If the connection has not been found add node to the open list
				else if ((!(cxn == -1))&&notFound(cxn)) {
					addedNodes[aNCounter] = cxn;
					aNCounter = aNCounter + 1;
				}
				//If the connection has been found check to see if path from current node is shorter
				else if ((!(cxn == -1)) && (notFound(cxn)==false)) {
					//If it is reassign parent
					if(getMoveDist(i, cxn)<nodeList[cxn]->getLevel()){
						nodeList[cxn]->setParent(i);
						nodeList[cxn]->setPriority(getMoveDist(i, cxn));
					}
				}
				//If the end of the connection list has been reached
 				else if(cxn == -1){
					//Remove current node from pq
					pq.pop();
					//Add all found connections to pq
					addNodes(i);
					break;
				}
			}
	}
}

//Return total distance from START node to node b through path of node a
int MainGame::getMoveDist(int a, int b) {
	int dX = nodeList[a]->getxPos() - nodeList[b]->getxPos();
	int dY = nodeList[a]->getyPos() - nodeList[b]->getyPos();
	int d =(sqrt(dX*dX + dY*dY));
	return nodeList[a]->getLevel()+d;
}

//If node has been found (a=array value of last node connected to finish node)
void MainGame::foundNode(int a) {
 
	//Clear all lines
	redrawFin();
	//Current node in path (starts from the end)
	int curNode = a;
	//Green
	SDL_SetRenderDrawColor(_renderer, 22, 204, 28, 255);
	//Record first node in path
	pathList += to_string(nodeList[curNode]->getArrayValue());
	pathList += " ";
	//Draw line from finish to connecting node
	SDL_RenderDrawLine(_renderer, nodeList[curNode]->getxPos(), nodeList[curNode]->getyPos(), endX, endY);
	//While not at the start node
	while (!(nodeList[curNode]->getParent() == -5)) {
		//Add the current node to the path record and draw line
		pathList += to_string(nodeList[curNode]->getParent());
		pathList += " ";
		SDL_RenderDrawLine(_renderer, nodeList[curNode]->getxPos(), nodeList[curNode]->getyPos(), nodeList[nodeList[curNode]->getParent()]->getxPos(), nodeList[nodeList[curNode]->getParent()]->getyPos());
		//Find the parent of the current node
		curNode = nodeList[curNode]->getParent();
	}
	//Display path record and draw path
	cout << "Path: " << pathList << endl;
	SDL_RenderPresent(_renderer);

}

//Returns true if node is not on the open nodes list(pq) or closed node list
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

//Adds nodes to the open node list (a=parent node)
void MainGame::addNodes(int a) {
	for (int i = 0; i < numNodes; i++) {
		//If there are no more added nodes, clear queue waitlist (addedNodes[]) and break
		if (addedNodes[i] == -1) {
			clearQueueList();
			break;
		}
		//Initialize node (set parent and priority), add to pq, and add to closed node list
		else {
			nodeList[addedNodes[i]]->setParent(a);
			nodeList[addedNodes[i]]->setPriority(getMoveDist(a, addedNodes[i]));
			pq.push(*nodeList[addedNodes[i]]);
			closedNodeList[closedCounter] = addedNodes[i];
			closedCounter = closedCounter + 1;				
		}
	}
}

//Reset addedNodes[]
void MainGame::clearQueueList() {
	aNCounter = 0;
	for (int i = 0; i < numNodes; i++) {
		addedNodes[i] = -1;
	}
}

//To clearly show path: clear all connection lines, but re-draw nodes and obstacles
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