/*
	CSC D84 - Unit 2 - MiniMax search and adversarial games

	This file contains stubs for implementing a MiniMax search
        procedure with alpha-beta pruning. Please read the assignment
	handout carefully - it describes the game, the data you will
	have to handle, and the search functions you must provide.

	Once you have read the handout carefully, implement your search
	code in the sections below marked with

	**************
	*** TO DO:
	**************

	Make sure to add it to your report.txt file - it will be marked!

	Have fun!

	DO NOT FORGET TO 'valgrind' YOUR CODE - We will check for pointer
	management being done properly, and for memory leaks.

	Starter code: F.J.E. Sep. 15
*/

#include "MiniMax_search.h"
#include "float.h"
#include "limits.h"

typedef struct Node{
	Node* next;
	double score;
	int origin;
}Node;

Node* newNode(double score, int origin){
	Node* newN = (Node*) calloc(1,sizeof(Node));
	if(newN == NULL){
		return NULL;
	}
	newN->score = score;
	newN->origin = origin;
	newN->next = NULL;
	return newN;
}

Node* prepend(Node* node, Node* list){
	node->next = list;
	return node;
}

Node* maxList(Node* start){
	Node* curr = start;
	Node* max = NULL;
	while(curr != NULL){
		if (max == NULL || curr->score > max->score){
			max = curr;
		}
		curr = curr->next;
	}
	return max;
}

Node* createPriorityNode(int origin, double score){
	return newNode(score, origin);
}

Node* minList(Node* start){
	Node* curr = start;
	Node* min = NULL;
	while(curr != NULL){
		if(min == NULL || curr->score < min->score){
			min = curr;
		}
		curr = curr->next;
	}
	return min;
}

void freeList(Node* start){
	Node* curr = start;
	Node* prev = start;
	while(curr != NULL){
		curr = curr->next;
		free(prev);
		prev = curr;
	}
	if(prev != NULL){
		free(prev);
	}
	return;
}

typedef struct Queue{
	Node* first;
	Node* last;
} Queue;

Queue* createQueue(){
	Queue* newQueue = (Queue*)calloc(1, sizeof(Queue));
	if(newQueue == NULL){
		// printf("Not enough memory for queue");
		return NULL;
	}
	newQueue->first = NULL;
	newQueue->last = NULL;

	return newQueue;
}

//enqueue into priority queue by sorting each time something is inserted into the queue
//slow but i didn't want to implement something like a min heap
void priorityEnQueue(Queue* queue, int value, int priority){
	Node* newNode = createPriorityNode(value, priority);
	if(queue->first == NULL){
		queue->first = newNode;
		queue->last = newNode;
		return;
	}
	//otherwise insert into linked list and sort it

	if(newNode->score <= queue->first->score){
		newNode->next = queue->first;
		queue->first = newNode;
		return;
	}

	Node* head = queue->first;
	//note prev should never be null since we always check to see if it should be inserted
	//at head first
	Node* prev = NULL;
	while(head != NULL){
		if(newNode->score <= head->score){
			prev->next = newNode;
			newNode->next = head;
			return;
		}
		prev = head;
		head = head->next;
	}
	//if iterated through entire list then it is the new last element
	prev->next = newNode;
	queue->last = newNode;
	return;
}

int deQueue(Queue* queue){
	if(queue->first == NULL){
		// printf("Nothing in queue");
		return -1;
	}
	Node* first = queue->first;
	queue->first = queue->first->next;
	if(queue->first == NULL){
		queue->last = NULL;
	}
	int val = first->origin;
	free(first);
	return val;
}

void freeQueue(Queue* queue){
	Node* first = queue->first;
	Node* temp = NULL;
	while(first != NULL){
		temp = first->next;
		free(first);
		first = temp;
	}
	queue->last = NULL;
	free(queue);
	return;
}

int get_grid_position(int coords[2]){
	return (coords[0] + (size_X * coords[1]));
}

int manhattan_dist(int point1[2], int point2[2]){
	return abs(point1[0] - point2[0]) + abs(point1[1] - point2[1]);
}

Node* permute_kitties(double gr[graph_size][4], int path[1][2], double minmax_cost[size_X][size_Y], int cat_loc[10][2], int cats, int cheese_loc[10][2], int cheeses, int mouse_loc[1][2], int mode, double (*utility)(int cat_loc[10][2], int cheese_loc[10][2], int mouse_loc[1][2], int cats, int cheeses, int depth, double gr[graph_size][4]), int agentId, int depth, int maxDepth, double alpha, double beta,
	int index, Node* scores){
	if(index == cats){
		//if reached end of array then we've achieved a permutation
		//run minimax and save that score
		double newScore = MiniMax(gr,path,minmax_cost,cat_loc,cats,cheese_loc,cheeses,mouse_loc,mode,utility,!agentId,depth+1,maxDepth,alpha,beta);
		//don't update minimax or anything yet because we have not moved the mouse
		//just append new score and return
		return prepend(newNode(newScore,get_grid_position(mouse_loc[0])),scores);
	}
	//otherwise we're still permuting
	//for each index in the array iterate through its possible values then permute the next ones
	for(int i = 0; i < 4; i++){
		int y_offset =  (1-(i%2)) * (-1+i);
		int x_offset = ((i%2) * (2 - i));

		if(gr[get_grid_position(cat_loc[index])][i] == 1
		&& cat_loc[index][0] + x_offset >= 0 && cat_loc[index][0] + x_offset < size_X
		&& cat_loc[index][1] + y_offset >= 0 && cat_loc[index][1] + y_offset < size_Y){
			
			//temporarily change the locations
			cat_loc[index][0] += x_offset;
			cat_loc[index][1] += y_offset;

			//permute the next kitty
			scores = permute_kitties(gr,path,minmax_cost,cat_loc,cats,cheese_loc,cheeses,mouse_loc,mode,utility,agentId,depth,maxDepth,
			alpha,beta,index+1,scores);

			//snap back to reality
			cat_loc[index][0] -= x_offset;
			cat_loc[index][1] -= y_offset;
			
		}
		//note if path not possible, we don't want to permute further anyways so can just skip
	}
	return scores;
}

double MiniMax(double gr[graph_size][4], int path[1][2], double minmax_cost[size_X][size_Y], int cat_loc[10][2], int cats, int cheese_loc[10][2], int cheeses, int mouse_loc[1][2], int mode, double (*utility)(int cat_loc[10][2], int cheese_loc[10][2], int mouse_loc[1][2], int cats, int cheeses, int depth, double gr[graph_size][4]), int agentId, int depth, int maxDepth, double alpha, double beta)
{
 /*
   This function is the interface between your solution for the assignment and the driver code. The driver code
   in MiniMax_search_core_GL will call this function once per frame, and provide the following data
   
   Board and game layout:

	Exactly the same as for Assignment 1 - have a look at your code if you need a reminder of how the adjacency
	list and agent positions are stored.	

	Note that in this case, the path will contain a single move - at the top level, this function will provide
	an agent with the 'optimal' mini-max move given the game state.


   IMPORTANT NOTE: Mini-max is a recursive procedure. This function will need to fill-in the mini-max values for 
	 	   all game states down to the maximum search depth specified by the user. In order to do that,
		   the function needs to be called with the correct state at each specific node in the mini-max
		   search tree.

		   The game state is composed of:

			* Mouse, cat, and cheese positions (and number of cats and cheeses)
			
		   At the top level (when this function is called by the mini-max driver code), the game state
		   correspond to the current situation of the game. But once you start recursively calling
		   this function for lower levels of the search tree the positions of agents will have changed.
		   
		   Therefore, you will need to define local variables to keep the game state at each node of the
		   mini-max search tree, and you will need to update this state when calling recursively so that
		   the search does the right thing.

		   This function *must check* whether:
			* A candidate move results in a terminal configuration (cat eats mouse, mouse eats cheese)
			  at which point it calls the utility function to get a value
	 		* Maximum search depth has been reached (depth==maxDepth), at which point it will also call
			  the utility function to get a value
			* Otherwise, call recursively using the candidate configuration to find out what happens
			  deeper into the mini-max tree.

   Arguments:
		gr[graph_size][4]   		- This is an adjacency list for the maze
		path[1][2] 			- Your function will return the optimal mini-max move in this array.
		minmax_cost[size_X][size_Y]	- An array in which your code will store the
						  minimax value for maze locations expanded by
						  the search *when called for the mouse, not
						  for the cats!*

						  This array will be used to provide a visual 
						  display of minimax values during the game.

		cat_loc[10][2], cats   - Location of cats and number of cats (we can have at most 10,
					 but there can be fewer). Only valid cat locations are 0 to (cats-1)
		cheese_loc[10][2], cheeses - Location and number of cheese chunks (again at most 10,
					     but possibly fewer). Valid locations are 0 to (cheeses-1)
		mouse_loc[1][2] - Mouse location - there can be only one!
		mode - Search mode selection:
					mode = 0 	- No alpha-beta pruning
					mode = 1	- Alpha-beta pruning

		(*utility)(int cat_loc[10][2], int cheese_loc[10][2], int mouse_loc[1][2], int cats, int cheeses, int depth, double gr[graph_size][4]);
				- This is a pointer to the utility function which returns a value for a specific game configuration

				   NOTE: Unlike the search assignment, this utility function also gets access to the graph so you can do any processing 					 that requires knowledge of the maze for computing the utility values.

				  * How to call the utility function from within this function : *
					- Like any other function:
						u = utility(cat_loc, cheese_loc, mouse_loc, cats, cheeses, depth, gr);
						
		agentId: Identifies which agent we are doing MiniMax for. agentId=0 for the mouse, agentId in [1, cats] for cats. Notice that recursive calls
                         to this function should increase the agentId to reflect the fact that the next level down corresponds to the next agent! For a game
                         with two cats and a mouse, the agentIds for the recursion should look like 0, 1, 2, 0, 1, 2, ...
	
		depth: Current search depth - whether this is a MIN or a MAX node depends both on depth and agentId.
		
		maxDepth: maximum desired search depth - once reached, your code should somehow return
			  a minimax utility value for this location.

		alpha. beta: alpha and beta values passed from the parent node to constrain search at this
			     level.

   Return values:
		Your search code will directly update data passed-in as arguments:
		
		- Mini-Max value	: Notice this function returns a double precision number. This is
					  the minimax value at this level of the tree. It will be used 
					  as the recursion backtracks filling-in the mini-max values back
					  from the leaves to the root of the search tree. 

		- path[1][2]		: Your MiniMax function will return the location for the agent's 
					  next location (i.e. the optimal move for the agent). 
		- minmax_cost[size_X][size_Y] 	:  Your search code will update this array to contain the
						   minimax value for locations that were expanded during
						   the search. This must be done *only* for the mouse.

						   Values in this array will be in the range returned by
						   your utility function.

		* Your code MUST NOT modify the locations or numbers of cats and/or cheeses, the graph,
	     	  or the location of the mouse - if you try, the driver code will know it *
			
		That's that, now, implement your solution!
 */

 /********************************************************************************************************
 * 
 * TO DO:	Implement code to perform a MiniMax search. This will involve a limited-depth BFS-like
 *              expansion. Once nodes below return values, your function will propagate minimax utilities
 *		as per the minimax algorithm.
 *	
 *		Note that if alpha-beta pruning is specified, you must keep track of alphas and betas
 *		along the path.
 *
 *		You can use helper functions if it seems reasonable. Add them to the MiniMax_search.h
 *		file and explain in your code why they are needed and how they are used.
 *
 *		Recursion should appear somewhere.
 *
 *		MiniMax cost: If the agentId=0 (Mouse), then once you have a MiniMax value for a location
 *		in the maze, you must update minmax_cost[][] for that location.
 *
 *		How you design your solution is up to you. But:
 *
 *		- Document your implementation by adding concise and clear comments in this file
 *		- Document your design (how you implemented the solution, and why) in the report
 *
 ********************************************************************************************************/

 // Stub so that the code compiles/runs - This will be removed and replaced by your code!
	if(depth >= maxDepth || checkForTerminal(mouse_loc,cat_loc,cheese_loc,cats,cheeses)){
		//once reach max depth, return a minimax value for current max depth
		double cost = utility(cat_loc,cheese_loc,mouse_loc,cats,cheeses,depth,gr);
		minmax_cost[mouse_loc[0][0]][mouse_loc[0][1]] = cost;
		//printf("depth %d, cost %lf, mode %d\n",depth,cost,agentId);
		return cost; 
	}
	//if not at max depth
	//then figure out if we're doining min (agent = cat = 1) or max (agent = mouse = 0)

	//TODO: STILL NEED TO UPDATE PATHS, PROPERLY
	//TODO: EXPAND THE TREE (for each of the possible moves of the mouse)
	//TODO: MOVE IT AND THEN RUN FOR ALL MOVES OF THE CATS (all children is seeing what direction a cat moves and what direction a mouse moves)


	Node* scores = NULL;
	
	//every child of the tree will be a combination of mouse/cat moves

	//permute every possible move from this level (for each of the mouse's directions, for each of the cat's directions)

	//if agent = 0 then mouse is moving, permute all possible children for the mouse
	if(agentId == 0){
		//permute all options for a mouse
		for(int i = 0; i < 4; i++){
			//if even will be moving up/down so y should be active

			//didn't want to do a bunch of iff statements like with A1 so devised this:
			//if 0 -1, if 2 +1 so it should be (1-(i%2)) * (-1+i) will be 1 * (-1 or 1) if even
			//odd will be 1 or 3 so it should be (i%2) * (-2 + i)
			int y_offset =  (1-(i%2)) * (-1+i);
			int x_offset = ((i%2) * (2 - i));

			if(gr[get_grid_position(mouse_loc[0])][i] == 1
			&& mouse_loc[0][0] + x_offset >= 0 && mouse_loc[0][0] + x_offset < size_X
			&& mouse_loc[0][1] + y_offset >= 0 && mouse_loc[0][1] + y_offset < size_Y){
				//if new position is valid
				
				//update positions and continue minimax with them
				mouse_loc[0][0] += x_offset;
				mouse_loc[0][1] += y_offset;
				//go through minimax to get cost for cats at lower depth
				double newCost = MiniMax(gr,path,minmax_cost,cat_loc,cats,cheese_loc,
				cheeses,mouse_loc,mode,utility,!agentId,depth+1,maxDepth,alpha,beta);
				//update minimax and scores
				minmax_cost[mouse_loc[0][0]][mouse_loc[0][1]] = newCost;
				scores = prepend(newNode(newCost,get_grid_position(mouse_loc[0])),scores);
				//return to normality
				mouse_loc[0][0] -= x_offset;
				mouse_loc[0][1] -= y_offset;
			}
		}
	}else{
		//permute all options for that cat (tricky since undefined number of cats)
		scores = permute_kitties(gr,path,minmax_cost,cat_loc,cats,cheese_loc,cheeses,mouse_loc,mode,
		utility,agentId,depth,maxDepth,alpha,beta,0,scores);
	}

	//get best option from available scores list
	Node* option;
	if(agentId == 0){
		//if mouse maximize utilities
		option = maxList(scores);
	}else{
		option = minList(scores);
	}
	//if there was no found min or max, return worst posible value and have mouse remain still
	if(option == NULL){
		freeList(scores);
		if(agentId == 0){
			path[0][0]=mouse_loc[0][0];
 			path[0][1]=mouse_loc[0][1];
			minmax_cost[mouse_loc[0][0]][mouse_loc[0][1]] = -DBL_MAX;
			return -DBL_MAX;
		}
		minmax_cost[mouse_loc[0][0]][mouse_loc[0][1]] = DBL_MAX;
		return DBL_MAX;
	}
	int origin = option->origin;
	double score = option->score;
	freeList(scores);
	//printf("Not null %d\n", option->origin);
	//update path for best node and return its score
	if(agentId == 0){
		path[0][0] = origin % size_X;
		path[0][1] = origin / size_Y;
	}
	//update minimax score
	minmax_cost[mouse_loc[0][0]][mouse_loc[0][1]] = score;
	//printf("depth %d, cost %lf, mode %d \n",depth,option->score,agentId);
	return score;
}

//Helper function used to determine if a pair of coordinates are in an array
//if they are in array, return its index, otherwise return -1
int coordsInArray(int coords ,int (*array)[2],int size){
	for(int i = 0; i < size; i++){
		if (coords == get_grid_position(*(array+i))){
			return i;
		}
	}
	return -1;
}

double heuristic(int x, int y, int cat_loc[10][2], int cheese_loc[10][2], int mouse_loc[1][2], int cats, int cheeses, double gr[graph_size][4])
{
 /*
	This function computes and returns the heuristic cost for location x,y.
	As discussed in lecture, this means estimating the cost of getting from x,y to the goal. 
	The goal is cheese. Which cheese is up to you.
	Whatever you code here, your heuristic must be admissible.

	Input arguments:

		x,y - Location for which this function will compute a heuristic search cost
		cat_loc - Cat locations
		cheese_loc - Cheese locations
		mouse_loc - Mouse location
		cats - # of cats
		cheeses - # of cheeses
		gr - The graph's adjacency list for the maze

		These arguments are as described in the search() function above
 */

	/**
	 * have each cost be manhattan distance to closest cheese (can figure out better algo later)
	*/
	int min_distance = INT_MAX;
	for(int i = 0; i < cheeses; i++){
		int new_dist = (abs(x - cheese_loc[i][0]) + abs(y - cheese_loc[i][1]));
		if (new_dist < min_distance){
			min_distance = new_dist;
		}
	}
	return min_distance;
}

int search_length(double gr[graph_size][4], int cat_loc[10][2], int cats, int cheese_loc[10][2], int cheeses, int mouse_loc[1][2]){

	//initalize priority queue
	//add starting node to priority queue
	Queue* priorityQueue = createQueue();
	int mouse_grid = get_grid_position(mouse_loc[0]);
	priorityEnQueue(priorityQueue,mouse_grid,1);

	int cheese_index = -1;
	int iteration = 0;

	int predecessor[graph_size];
	int distances[graph_size] = {0};

	//initalizing the predecessor arr
	for(int i = 0; i < graph_size; i++){
		predecessor[i] = -1;
	}

	//while priority queue has an entry
	while(priorityQueue->first != NULL){

		//pop off from priority queue
		int curr_pos = deQueue(priorityQueue);
		// printf("dequeued %d\n",curr_pos);
		int curr_x = curr_pos % size_X;
		int curr_y = curr_pos / size_Y;
		//if its a cheese then you've found the cheese
		cheese_index = coordsInArray(curr_pos,cheese_loc,cheeses);
		if(cheese_index >= 0){
			// printf("cheese has been found!\n");
			break;
		}

		
		//otherwise see if other nodes are valid and put them in the priority queue based off their heuristic

		if(gr[curr_pos][0] == 1 && coordsInArray(curr_pos-size_X,cat_loc,cats) < 0 && curr_pos - size_X >= 0 && predecessor[curr_pos-size_X] == -1){
			int new_pos = curr_pos-size_X;
			distances[new_pos] = distances[curr_pos] + 1;
			predecessor[new_pos] = curr_pos;
			priorityEnQueue(priorityQueue, new_pos, distances[curr_pos] + heuristic(new_pos % size_X,(int)new_pos / size_Y,cat_loc,cheese_loc,mouse_loc,cats,cheeses,gr));
		}
		if(gr[curr_pos][1] == 1 && coordsInArray(curr_pos+1,cat_loc,cats) < 0 && curr_pos+1 < graph_size  && predecessor[curr_pos+1] == -1){
			int new_pos = curr_pos+1;
			distances[new_pos] = distances[curr_pos] + 1;
			predecessor[new_pos] = curr_pos;
			priorityEnQueue(priorityQueue, new_pos, distances[curr_pos] + heuristic(new_pos % size_X,(int)new_pos / size_Y,cat_loc,cheese_loc,mouse_loc,cats,cheeses,gr));
		}
		if(gr[curr_pos][2] == 1 && coordsInArray(curr_pos+size_X,cat_loc,cats) < 0 && curr_pos + size_X < graph_size && predecessor[curr_pos+size_X] == -1){
			int new_pos = curr_pos+size_X;
			distances[new_pos] = distances[curr_pos] + 1;
			predecessor[new_pos] = curr_pos;
			priorityEnQueue(priorityQueue, new_pos, distances[curr_pos] + heuristic(new_pos % size_X,(int)new_pos / size_Y,cat_loc,cheese_loc,mouse_loc,cats,cheeses,gr));
		}
		if(gr[curr_pos][3] == 1 && coordsInArray(curr_pos-1,cat_loc,cats) < 0 && curr_pos -1 >= 0  && predecessor[curr_pos-1] == -1){
			int new_pos = curr_pos-1;
			distances[new_pos] = distances[curr_pos] + 1;
			predecessor[new_pos] = curr_pos;
			priorityEnQueue(priorityQueue, new_pos, distances[curr_pos] + heuristic(new_pos % size_X,(int)new_pos / size_Y,cat_loc,cheese_loc,mouse_loc,cats,cheeses,gr));
		}

		iteration++;
		
	}
	//if don't have a cheese index then path wasn't found
	if (cheese_index < 0){
		// printf("no cheese found\n");
		freeQueue(priorityQueue);
		return -1;
	}
	
	//otherwise need to backtrack the path

	int curr_cords = get_grid_position(cheese_loc[cheese_index]);
	int path_length = 0;
	while(curr_cords != mouse_grid){
		//add the current cords to the path (by operating on its grid value)
		//then go to its predessecor

		curr_cords = predecessor[curr_cords];
		path_length++;
	}
	//free the queue

	freeQueue(priorityQueue);
	return path_length;

}

double utility(int cat_loc[10][2], int cheese_loc[10][2], int mouse_loc[1][2], int cats, int cheeses, int depth, double gr[graph_size][4])
{
 /*
	This function computes and returns the utility value for a given game configuration.
	As discussed in lecture, this should return a positive value for configurations that are 'good'
	for the mouse, and a negative value for locations that are 'bad' for the mouse.

	How to define 'good' and 'bad' is up to you. Note that you can write a utility function
	that favours your mouse or favours the cats, but that would be a bad idea... (why?)

	Input arguments:

		cat_loc - Cat locations
		cheese_loc - Cheese locations
		mouse_loc - Mouse location
		cats - # of cats
		cheeses - # of cheeses
		depth - current search depth
		gr - The graph's adjacency list for the maze

		These arguments are as described in A1. Do have a look at your solution!
 */


	//WIN CONDITIONS: ALL CATS DEAD OR ALL CHEESE EATEN
	//LOSE CONDITIONS: MOUSE TRAPPED OR CAT EATS MOUSE

	//CAT EATING MOUSE SHOULD BE WORST POSSIBLE SCORE ALONGSIDE TRAPPINg
	// for(int i = 0; i < cats; i++){
	// 	if(manhattan_dist(cat_loc[i],mouse_loc[0]) == 0){
	// 		return -INT_MAX;
	// 	}
	// }

	return (size_X * size_Y) - search_length(gr,cat_loc,cats,cheese_loc,cheeses,mouse_loc);
}

int checkForTerminal(int mouse_loc[1][2],int cat_loc[10][2],int cheese_loc[10][2],int cats,int cheeses)
{
 /* 
   This function determines whether a given configuration constitutes a terminal node.
   Terminal nodes are those for which:
     - A cat eats the mouse
     or
     - The mouse eats a cheese
   
   If the node is a terminal, the function returns 1, else it returns 0
 */

 // Check for cats having lunch
 for (int i=0; i<cats; i++)
  if (mouse_loc[0][0]==cat_loc[i][0]&&mouse_loc[0][1]==cat_loc[i][1]) return(1);

 // Check for mouse having lunch
 for (int i=0; i<cheeses; i++)
  if (mouse_loc[0][0]==cheese_loc[i][0]&&mouse_loc[0][1]==cheese_loc[i][1]) return(1);

 return(0);

}

