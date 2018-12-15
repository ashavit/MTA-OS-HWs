#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>    // execvp(), pipe()
#include "PolygonInterface.h"
#include "Workers.h"

#define IS_BIT_I_SET(NUM, I)         ( (NUM >> I) & 1 )
#define IS_NEW_POLYGON(NUM)          IS_BIT_I_SET((NUM), 1)
#define IS_NEW_SQUARE(NUM)             IS_BIT_I_SET((NUM), 2)
#define OUTPUT_PARAMS(NUM)           ( (NUM >> 3) & 7 )
#define OUTPUT_OBJECT_TYPE(NUM)      ( (NUM >> 6) & 3 )

#define N_SIZE 100
#define WORKERS_COUNT 2

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

void printOutputCommand(enum OUTPUT_OBJ, int params);
void printSingleOutputCommand(enum POLYGON shape, long long unsigned coords, int params);
void sendToWriter(char* str, size_t len);

List globalPolyList;
func_ptr funcPtrArr[FUNC_ARR_SIZE] = {add_polygon, print_polygon, print_perimeter, print_area};

int pipe_reader[2];
int pipe_writer[2];

void loadInterface() {

    if (pipe(pipe_reader) < 0 || pipe(pipe_writer) < 0) {
        fprintf(stderr, "cannot open pipe\n");
        exit(PIPE_CREATION_ERROR);
    }

    if (fork() == 0) {
        close(pipe_writer[0]);
        close(pipe_writer[1]);
        do_reader(pipe_reader);
    } else if (fork() == 0) {
        close(pipe_reader[0]);
        close(pipe_reader[1]);
        do_writer(pipe_writer);
    }

    // Parent thread - only read from reader pipe, write to writer pipe
    close(pipe_reader[1]);
    close(pipe_writer[0]);

    globalPolyList = makeEmptyList();

    BOOL isNewObject = FALSE;
    BOOL isNewSquare = FALSE;

    int rBytes, wBytes = 0;
    long long inputCommand = 0;
    long long inputPoly = 0;

    while ((rBytes = read(pipe_reader[0], &inputCommand, sizeof(inputCommand))) > 0) {
        // printf("main got input command: %llx (read len = %d)\n", inputCommand, rBytes);
        isNewObject = IS_NEW_POLYGON(inputCommand);

        if (isNewObject) {
            isNewSquare = IS_NEW_SQUARE(inputCommand);

            // Read coordinates form pipe
            if ((rBytes = read(pipe_reader[0], &inputPoly, sizeof(inputPoly))) <= 0)
                break;

            // printf("main got input coords: %llx (read len = %d)\n", inputPoly, rBytes);
            funcPtrArr[FUNC_ADD_POLYGON](TRIANGLE + isNewSquare, inputPoly);
        }

        printOutputCommand(OUTPUT_OBJECT_TYPE(inputCommand), OUTPUT_PARAMS(inputCommand));
    }

    if (rBytes == -1) {
        printf("MAIN: Error reading from pipe");
        exit(PIPE_READ_ERROR);
    } else if (wBytes == -1) {
        printf("MAIN: Error writing to pipe");
        exit(PIPE_WRITE_ERROR);
    }

    // No need for anymore pipes
    close(pipe_reader[0]);
    close(pipe_writer[1]);

    freeList(globalPolyList);

    // Wait for child threads to end
    for (int i = 0; i < WORKERS_COUNT; ++i)
      wait(NULL);
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

// Expect length of message including '\0' (end of line)
void sendToWriter(char* str, size_t len) {
    int wBytes = write(pipe_writer[1], &len, sizeof(len));
    if (wBytes == -1) {
        printf("MAIN: Error writing to pipe");
        exit(PIPE_WRITE_ERROR);
    }

    wBytes = write(pipe_writer[1], str, len);
    if (wBytes == -1) {
        printf("MAIN: Error writing to pipe");
        exit(PIPE_WRITE_ERROR);
    }

    // printf("write to pipe_writer buff=%s and len=%d\n", str, wBytes);
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
    char buff[N_SIZE];
    char temp[N_SIZE];
    Point* points = convertCoordsToPoints(coords, shape);

    sprintf(buff, "%s", (TRIANGLE == shape ? "triangle" : "square"));
    for (int i = 0; i < shape; ++i) {
        sprintf(temp, " {%d, %d}", points[i].x, points[i].y);
        strcat(buff, temp);
    }
    strcat(buff, "\n");

    sendToWriter(buff, strlen(buff) + 1);
    free(points);
}

void print_perimeter(enum POLYGON shape, long long unsigned coords) {
    char buff[N_SIZE];
    float perimeter = 0;
    float* vertices = convertCoordsToVertices(coords, shape);

    for (int i = 0; i < shape; ++i) {
        perimeter += vertices[i];
    }

    sprintf(buff, "perimeter = %.1f\n", perimeter);
    sendToWriter(buff, strlen(buff) + 1);
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

    char buff[N_SIZE];
    sprintf(buff, "area = %.1f\n", area);
    sendToWriter(buff, strlen(buff) + 1);
    free(points);
}
