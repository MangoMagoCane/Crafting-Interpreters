#ifndef clox_line_h
#define clox_line_h

#include "common.h"

typedef struct {
    int number;
    int occurrence;
} Line;

typedef struct {
    int count;
    int capacity;
    Line *lines;
} LineArray;

void initLineArray(LineArray *array);
void writeLineArray(LineArray *array, int line);
void freeLineArray(LineArray *array);
void printLineArray(LineArray *array);
int getLine(LineArray *array, int index);

#endif // clox_line_h
