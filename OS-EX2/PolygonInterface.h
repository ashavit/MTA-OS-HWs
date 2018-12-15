#ifndef PolygonInterface_h
#define PolygonInterface_h

#include <stdio.h>
#include "Commons.h"

enum POLYGON {TRIANGLE=3, SQUARE=4};
typedef struct listNode {
    enum POLYGON poly;
    long long unsigned polygon;
    struct listNode* next;
} ListNode;

typedef struct list
{
    ListNode* head;
    ListNode* tail;
} List;

void loadInterface(void);


/// These functions signitures are given

#define FUNC_ADD_POLYGON 0
#define FUNC_PRINT_POLYGON 1
#define FUNC_PERIMETER 2
#define FUNC_AREA 3
#define FUNC_ARR_SIZE 4

typedef void(*func_ptr)(enum POLYGON, long long unsigned);

// add new polygon to the list
void add_polygon(enum POLYGON, long long unsigned);

// print the type of polygon and its vertices
void print_polygon(enum POLYGON, long long unsigned);

// calculate and print the perimeter
void print_perimeter(enum POLYGON, long long unsigned);

// calculate and print the area
void print_area(enum POLYGON, long long unsigned);

#endif /* PolygonInterface_h */
