#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "PolygonInterface.h"

#define IS_BIT_I_SET(NUM, I)         ( (NUM >> I) & 1 )
#define IS_FINAL_COMMAND(NUM)        IS_BIT_I_SET((NUM), 0)
#define IS_NEW_POLYGON(NUM)          IS_BIT_I_SET((NUM), 1)
#define IS_NEW_SQUARE(NUM)             IS_BIT_I_SET((NUM), 2)
#define OUTPUT_PARAMS(NUM)           ( (NUM >> 3) & 7 )
#define OUTPUT_OBJECT_TYPE(NUM)      ( (NUM >> 6) & 3 )

typedef struct point {
    signed char x;
    signed char y;
} Point;

enum OUTPUT_OBJ {CURRENT=0, ALL_TRI=1, ALL_SQUARES=2, ALL=3 };

List makeEmptyList(void);
BOOL isEmptyList(List lst);
ListNode* createNewPolyNode(enum POLYGON shape, long long unsigned coords,ListNode * next);
void insertNodeToEndListNode(List* lst, ListNode * tail);
void freeList(List lst);

long long unsigned hexStringValue(char*);
void printInput(long long input);
void printOutputCommand(enum OUTPUT_OBJ, int params);
void printSingleOutputCommand(enum POLYGON shape, long long unsigned coords, int params);

List globalPolyList;
func_ptr funcPtrArr[FUNC_ARR_SIZE] = {add_polygon, print_polygon, print_perimeter, print_area};

void loadInterface() {
    globalPolyList = makeEmptyList();

    BOOL isFinal = FALSE;
    BOOL isNewObject = FALSE;
    BOOL isNewSquare = FALSE;

    char input[17];
    long long inputCommand = 0;
    long long inputPoly = 0;

    while (TRUE) {
        // printf("Input Command:\n");
        scanf("%s", input);
        // printf("\n");

        inputCommand = hexStringValue(input);
//        printInput(inputCommand);

        isFinal = IS_FINAL_COMMAND(inputCommand);
        isNewObject = IS_NEW_POLYGON(inputCommand);

        if (isNewObject) {
            isNewSquare = IS_NEW_SQUARE(inputCommand);

            // printf("Input %s coordinates:\n", (isNewSquare ? "square" : "triangle"));
            scanf("%s", input);
            // printf("\n");

            inputPoly = hexStringValue(input);
            funcPtrArr[FUNC_ADD_POLYGON](TRIANGLE + isNewSquare, inputPoly);
        }

        printOutputCommand(OUTPUT_OBJECT_TYPE(inputCommand), OUTPUT_PARAMS(inputCommand));

        if (TRUE == isFinal)
            break;
    }

    freeList(globalPolyList);
}

List makeEmptyList() {
    List res;
    res.head = res.tail = NULL;
    return res;
}

BOOL isEmptyList(List lst) {
    return (lst.head == NULL);
}

ListNode* createNewPolyNode(enum POLYGON shape, long long unsigned coords,ListNode * next) {
    ListNode* res = (ListNode*)malloc(sizeof(ListNode));
    res->poly = shape;
    res->polygon = coords;
    res->next = next;

    return res;
}

void insertNodeToEndListNode(List* lst, ListNode * newTail) {
    if (isEmptyList(*lst)) {
        lst->head = lst->tail = newTail;
    } else {
        lst->tail->next = newTail;
        lst->tail = newTail;
    }
}

void freeList(List lst) {
    ListNode* curr = lst.head;
    ListNode* saver = NULL;

    while (curr != NULL)
    {
        saver = curr->next;
        free(curr);
        curr = saver;
    }
}

long long unsigned hexStringValue(char* str) {
    unsigned long long res = strtoull(str, NULL, 16);
    return res;
}

void printInput(long long input) {
    printf("input = %lld\n", input);

    int loops = sizeof(input) * 8;
    for (int j = 0; j < loops; ++j) {
        printf("%d", (int)( (input >> j) & 1 ));
    }
    printf("\n");
}

void printOutputCommand(enum OUTPUT_OBJ type, int params) {
    if (isEmptyList(globalPolyList)) return;
    if (CURRENT == type) {
        printSingleOutputCommand(globalPolyList.tail->poly, globalPolyList.tail->polygon, params);
        return;
    }

    int deltaTriangleToAllTri = TRIANGLE - ALL_TRI;
    ListNode *curr = globalPolyList.head;
    while (curr != NULL)
    {
        if (ALL == type || (deltaTriangleToAllTri + type == curr->poly)) {
            printSingleOutputCommand(curr->poly, curr->polygon, params);
        }
        curr = curr->next;
    }
    // printf("\n");
}

void printSingleOutputCommand(enum POLYGON shape, long long unsigned coords, int params) {
    if (IS_BIT_I_SET(params, 0)) {
        funcPtrArr[FUNC_PRINT_POLYGON](shape, coords);
    }
    if (IS_BIT_I_SET(params, 1)) {
        funcPtrArr[FUNC_PERIMETER](shape, coords);
    }
    if (IS_BIT_I_SET(params, 2)) {
        funcPtrArr[FUNC_AREA](shape, coords);
    }
    // printf("\n");
}

Point* convertCoordsToPoints(unsigned long long coords, enum POLYGON shape) {
    Point* points = (Point*)malloc(shape * sizeof(Point));
    if (!points) {
        printf("Malloc error");
        exit(MALLOC_ERROR);
    }

    // Every coordinate is 2 bytes, x is LSB
    for (int i = 0; i < shape; ++i) {
        Point* p = &points[i];
        p->x = ((coords >> 8*2*i) & 255);
        p->y = ((coords >> (8*2*i + 8)) & 255);
    }
    return points;
}

float* convertPointsToVertices(Point* points, enum POLYGON shape) {
    float* vertices = (float*)malloc(shape * sizeof(float));
    if (!vertices) {
        printf("Malloc error");
        exit(MALLOC_ERROR);
    }

    Point p1, p2;
    float v = 0;

    for (int i = 0; i < shape; ++i) {
        p1 = points[i];
        p2 = points[ (i+1) % shape ];

        v = sqrt((double)(p1.x-p2.x) * (p1.x-p2.x) + (p1.y-p2.y) * (p1.y-p2.y));
        vertices[i] = v;
    }

    return vertices;
}

float* convertCoordsToVertices(unsigned long long coords, enum POLYGON shape) {
    Point* points = convertCoordsToPoints(coords, shape);
    float* res = convertPointsToVertices(points, shape);
    free(points);
    return res;
}

// Using Heron's formula
float calculateTriangeAreaByVertices(float a, float b, float c) {
    float s = (a + b + c)/2; // the semiperimeter = half of the triangle's perimeter
    float res = sqrtf(s * (s - a) * (s - b) * (s - c));
    return res;
}

/// These functions signitures are given

void add_polygon(enum POLYGON shape, long long unsigned coords) {
    ListNode* newPoly = createNewPolyNode(shape, coords, NULL);
    insertNodeToEndListNode(&globalPolyList, newPoly);
}

void print_polygon(enum POLYGON shape, long long unsigned coords) {
    Point* points = convertCoordsToPoints(coords, shape);

    printf("%s", (TRIANGLE == shape ? "triangle" : "square"));
    for (int i = 0; i < shape; ++i) {
        printf(" {%d, %d}", points[i].x, points[i].y);
    }
    printf("\n");
    free(points);
}

void print_perimeter(enum POLYGON shape, long long unsigned coords) {
    float perimeter = 0;
    float* vertices = convertCoordsToVertices(coords, shape);

    for (int i = 0; i < shape; ++i) {
        perimeter += vertices[i];
    }

    printf("perimeter = %.1f\n", perimeter);
    free(vertices);
}

void print_area(enum POLYGON shape, long long unsigned coords) {
    float area = 0;
    float* vertices;
    Point* points = convertCoordsToPoints(coords, shape);

    vertices = convertPointsToVertices(points, TRIANGLE);
    area = calculateTriangeAreaByVertices(vertices[0], vertices[1], vertices[2]);
    free(vertices);

    if (SQUARE == shape) {
        points[1] = points[3];
        vertices = convertPointsToVertices(points, TRIANGLE);
        area += calculateTriangeAreaByVertices(vertices[0], vertices[1], vertices[2]);
        free(vertices);
    }

    printf("area = %.1f\n", area);
    free(points);
}
