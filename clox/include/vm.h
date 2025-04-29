#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stackTop;
} VM;

void initVM(VM *vm);
void freeVM(VM *vm);
InterpretResult interpret(VM *vm, const char *source);
void push(VM *vm, Value value);
Value pop(VM *vm);

#endif // clox_vm_h
