#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

static void resetStack(VM *vm) {
    vm->stackTop = vm->stack;
}

void initVM(VM *vm) {
    resetStack(vm);
}

void freeVM(VM *vm) {
    UNUSED(vm);
}

void push(VM *vm, Value value) {
    *vm->stackTop = value;
    vm->stackTop++;
}

Value pop(VM *vm) {
    vm->stackTop--;
    return *vm->stackTop;
}

static InterpretResult run(VM *vm) {
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG() \
    do { \
        int i = READ_BYTE(); \
        i += READ_BYTE() << 8; \
        i += READ_BYTE() << 16; \
        constant = i; \
    } while (false);
#define BINARY_OP(op) \
    do { \
        double b = pop(vm); \
        double a = pop(vm); \
        push(vm, a op b); \
    } while (false);

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        for (Value *slot = vm->stack; slot < vm->stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf("] ");
        }
        printf("\n");
        disassembleInstruction(vm->chunk, (vm->ip - vm->chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
        case OP_CONSTANT:
            Value constant = READ_CONSTANT();
            push(vm, constant);
            break;
        case OP_CONSTANT_LONG:
            READ_CONSTANT_LONG();
            push(vm, constant);
            break;
        case OP_NEGATE: push(vm, -pop(vm)); break;
        case OP_ADD:      BINARY_OP(+); break;
        case OP_SUBTRACT: BINARY_OP(-); break;
        case OP_MULTIPLY: BINARY_OP(*); break;
        case OP_DIVIDE:   BINARY_OP(/); break;
        case OP_RETURN:
            printValue(pop(vm));
            printf("\n");
            return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(VM *vm, const char *source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm->chunk = &chunk;
    vm->ip = vm->chunk->code;

    InterpretResult result = run(vm);

    freeChunk(&chunk);
    return result;
}
