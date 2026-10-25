enum Opcode
{
    OP_HLT      = 0,
    OP_PUSH     = 1,
    OP_ADD      = 2,
    OP_SUB      = 3,
    OP_DIV      = 4,
    OP_OUT      = 5,
    OP_POP      = 6,
    OP_JMP      = 7,
    OP_MUL      = 8,
    OP_JA       = 9,
    OP_PUSHr    = 10,
    OP_CALL     = 11,
    OP_RET      = 12,
    OP_PUSHm    = 13,
    OP_POPm     = 14,
    OP_PUSHmr   = 15,
    OP_POPmr    = 16,
};

#define SIZE_OPCODE 17
#define SIZE_BUF    256
#define LABEL_MAX   5
