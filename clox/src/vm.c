#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

static void resetStack(VM *vm) {
    vm->stackTop = vm->stack;
}

static void runtimeError(VM *vm, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm->ip - vm->chunk->code - 1;
    int line = getLine(&vm->chunk->lineNumbers, instruction);
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack(vm);
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

static double factorial(double n) {
    return n < 2 ? 1 : n * factorial(n - 1);
}

static Value peek(VM *vm, int distance) {
    return vm->stackTop[-1 - distance];
}

static InterpretResult run(VM *vm) {
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT(index) (vm->chunk->constants.values[index])
#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) { \
            runtimeError(vm, "Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop(vm)); \
        double a = AS_NUMBER(pop(vm)); \
        push(vm, valueType(a op b)); \
    } while (false)

#ifdef DEBUG_TRACE_EXECUTION
        for (Value *slot = vm->stack; slot < vm->stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf("] ");
        }
        printf("\n");
        disassembleInstruction(vm->chunk, (vm->ip - vm->chunk->code));
#endif

    for (;;) {
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
        case OP_CONSTANT:
            Value constant = READ_CONSTANT(READ_BYTE());
            push(vm, constant);
            break;
        case OP_CONSTANT_LONG:
            int i = READ_BYTE();
            i += READ_BYTE() << 8;
            i += READ_BYTE() << 16;
            push(vm, READ_CONSTANT(i));
            break;
        case OP_NEGATE:
            if (!IS_NUMBER(peek(vm, 0))) {
                runtimeError(vm, "Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(vm, NUMBER_VAL(-AS_NUMBER(pop(vm))));
            break;
        case OP_FACTORIAL:
            if (!IS_NUMBER(peek(vm, 0))) {
                runtimeError(vm, "Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(vm, NUMBER_VAL(factorial(AS_NUMBER(pop(vm)))));
            break;
        case OP_EXPONENTIATE:
            if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {
                runtimeError(vm, "Operands must be numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }
            double b = AS_NUMBER(pop(vm));
            double a = AS_NUMBER(pop(vm));
            push(vm, NUMBER_VAL(pow(a, b)));
            break;
        case OP_ADD:      BINARY_OP(NUMBER_VAL, +); break;
        case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
        case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
        case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
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
