
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct NODE{
	//todo: define struct NODE
    char* name;
	struct NODE* parent;
	struct NODE* child;
	struct NODE* prev;
	struct NODE* next;
}NODE;


//MakeNode: make a new node
NODE* MakeNode(char* name){
	//todo	
	NODE* newNode = (NODE*)malloc(sizeof(NODE));
	newNode->name = (char*)malloc(strlen(name) + 1);
	strcpy(newNode->name, name);
	newNode -> parent = NULL;
	newNode -> child = NULL;
	newNode -> prev = NULL;
	newNode -> next = NULL;
	return newNode;
}


// Insert node (parent-child): append a child node to parent node
void InsertChild(NODE* parent_node, NODE* this_node){
	//todo
	parent_node->child = this_node;
    this_node->parent = parent_node;
}


// Insert node (prev-next): append a next node to prev node 
void InsertSibling(NODE* prev_node, NODE* this_node){
	//todo
	prev_node -> next = this_node;
	this_node -> prev = prev_node;
}

//Tree walk algorithm
void WalkTree(NODE* node){
	//todo
	if(node == NULL) return;

	printf("(%s", node->name);

	// node has a child 
	if(node->child != NULL) {
		printf("\n");
		WalkTree(node->child);
	} 

	// node does not have a child 
	if(node->next != NULL) {
		printf(")\n");
		WalkTree(node->next);
	} else {
		printf(")");
	}
}

// int main(void) {
// 	NODE* A = MakeNode("A");
// 	NODE* B = MakeNode("B");
// 	NODE* C = MakeNode("C");
// 	NODE* D = MakeNode("D");
// 	NODE* E = MakeNode("E");
// 	NODE* F = MakeNode("F");
// 	NODE* G = MakeNode("G");
// 	NODE* H = MakeNode("H");
// 	NODE* I = MakeNode("I");

// 	InsertChild(A, B);
// 	InsertSibling(B, E);
// 	InsertSibling(E, I);
// 	InsertChild(B, C);
// 	InsertSibling(C, D);
// 	InsertChild(E, F);
// 	InsertSibling(F, G);
// 	InsertSibling(G, H);

// 	WalkTree(A);
// }


