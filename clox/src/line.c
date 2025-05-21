#include <stdio.h>

#include "memory.h"
#include "line.h"

void initLineArray(LineArray *array) {
    array->count = 0;
    array->capacity = 0;
    array->lines = NULL;
}

void writeLineArray(LineArray *array, int line) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->lines = GROW_ARRAY(Line, array->lines,
                oldCapacity, array->capacity);
    }

    if (array->count > 0 &&
            line == array->lines[array->count - 1].number) {
        array->lines[array->count - 1].occurrence++;
        return;
    }

    array->lines[array->count].number = line;
    array->lines[array->count].occurrence = 1;
    array->count++;
}

void freeLineArray(LineArray *array) {
    FREE_ARRAY(Line, array->lines, array->capacity);
    initLineArray(array);
}

void printLineArray(LineArray *array) {
    for (int i = 0; i < array->count; i++) {
        printf("%-8d  %8d\n", array->lines[i].number, array->lines[i].occurrence);
    }
}

int getLine(LineArray *array, int index) {
    int i = 0;

    index++;
    do {
        if (i >= array->count) {
            return -1;
        }
        index -= array->lines[i++].occurrence;
    } while (index > 0);

    return array->lines[i - 1].number;
}
