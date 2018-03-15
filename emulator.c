#include <stdio.h>
#include <stdlib.h>

typedef struct ConditionCodes {
    uint8_t     z:1;
    uint8_t     s:1;
    uint8_t     p:1;
    uint8_t     cy:1;
    uint8_t     ac:1;
    uint8_t     pad:3;
} ConditionCodes;

typedef struct State8080 {
    uint8_t     a;
    uint8_t     b;
    uint8_t     c;
    uint8_t     d;
    uint8_t     e;
    uint8_t     h;
    uint8_t     l;
    uint16_t    sp;
    uint16_t    pc;
    uint8_t     *memory;
    struct      ConditionCodes      cc;
    uint8_t     int_enable;
} State8080;

void UnimplementedInstruction(State8080* state)
{
    // pc will have advanced one, so undo that
    printf("Error: Unimplemented instruction\n");
    exit(1);
}

int Emulate8080Op(State8080* state)
{
    unsigned char *opcode = &state->memory[state->pc];

    switch(*opcode)
    {
        case 0x00: break;                                   // NOP
        case 0x01: 
            state->c = opcode[1];
            state->b = opcode[2];
            state->pc += 2;
            break;
        case 0x02: UnimplementedInstruction(state); break;    
        case 0x03: UnimplementedInstruction(state); break;    
        case 0x04: UnimplementedInstruction(state); break;    
        
        case 0x41: state->b = state->c; break;              // MOV B,C
        case 0x42: state->b = state->d; break;              // MOV B,D
        case 0x43: state->b = state->e; break;              // MOV B,E

        case 0x80:          // ADD B
        {
            // do the math with higher precision so we can capture the carry out
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->b;

            // zero flag
            if ((answer & 0xff) == 0)
                state->cc.z = 1;
            else
                state->cc.z = 0;

            // sign flag
            if (answer & 0x80)
                state->cc.s = 1;
            else
                state->cc.s = 0;

            // carry flag
            if (answer > 0xff)
                state->cc.cy = 1;
            else
                state->cc.cy = 0;

            // parity is handled by a subroutine
            state->cc.p = Parity(answer & 0xff);

            state->a = answer & 0xff;
        }

            break;
        case 0x81:      // ADD C
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->c;    
            state->cc.z = ((answer & 0xff) == 0);    
            state->cc.s = ((answer & 0x80) != 0);    
            state->cc.cy = (answer > 0xff);    
            state->cc.p = Parity(answer&0xff);    
            state->a = answer & 0xff;
        }

            break;

        case 0x86:      //ADD M    
        {
            uint16_t offset = (state->h<<8) | (state->l);
            uint16_t answer = (uint16_t) state->a + state->memory[offset];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.cy = (answer > 0xff);
            state->cc.p = Parity(answer & 0xff);
            state->a = answer & 0xff;
        }

            break;

        case 0xC1:      // POP B
        {
            state->c = state->memory[state->sp];
            state->b = state->memory[state->sp + 1];
            state->sp += 2;
        }
            break;

        case 0xC5:      // PUSH B
        {
            state->memory[state->sp-1] = state->b;
            state->memory[state->sp-2] = state->c;
            state->sp -= 2;
        }
            break;

        case 0xC2:      // JNZ address
        {
            if (0 == state->cc.z)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                state->pc += 2;
        }
            
            break;

        case 0xC3:      // JMP address
        {
            state->pc = (opcode[2] << 8) | opcode[1];
        }
            break;

        case 0xC3:      // CALL address
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp - 1] = (ret >> 8) & 0xff;
            state->memory[state->xp - 2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
        }

            break;

        case 0xC9:      // RET
        {
            state->pc = state->memory[state->pc] | (state->memory[state->sp + 1] << 8);
            state->sp += 2;
        }

            break;

        case 0xC6:      //ADI byte
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) opcode[1];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) != 0);
            state->cc.cy = (answer > 0xff);
            state->cc.p = Parity(answer&0xff);
            state->a = answer & 0xff;
        }

            break;

        case 0xfe: UnimplementedInstruction(state); break;    
        case 0xff: UnimplementedInstruction(state); break;
    }
    state->pc += 1;
}

int main(int argc, char** argv)
{
    // add code here to read in ROM and feed it to our emulator
    return 0;
}
