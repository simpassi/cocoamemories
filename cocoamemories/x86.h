//
//  x86.h
//  cocoamemories
//
//  Created by Virokannas, Simo on 6/8/20.
//  Copyright © 2020 IHIHI. All rights reserved.
//

#ifndef x86_h
#define x86_h

#include <stdio.h>

// A simple registry structure to get access to the 8/16 bit registers in tandem
typedef struct {
    union {
        uint16_t ax;
        struct {
            uint8_t al;
            uint8_t ah;
        };
    };
    union {
        uint16_t bx;
        struct {
            uint8_t bl;
            uint8_t bh;
        };
    };
    union {
        uint16_t cx;
        struct {
            uint8_t cl;
            uint8_t ch;
            
        };
    };
    union {
        uint16_t dx;
        struct {
            uint8_t dl;
            uint8_t dh;
        };
    };
    uint16_t es;
    uint16_t bp;
    uint16_t si;
    uint16_t di;
    uint16_t swp;
    uint8_t sf;
    uint8_t cf;
    uint8_t zf;
    float st0;
    uint8_t mem[1024];
} x86_reg;

extern x86_reg r;

typedef struct {
    void (*func8)(uint8_t *, uint8_t *);
    void (*func16)(uint16_t *, uint16_t *);
} x86_instr;

void mov8(uint8_t *a, uint8_t *b);
void mov16(uint16_t *a, uint16_t *b);

// implement some of the x86 functions in pseudo-register code

// div8 is usually provided by the compiler, with O1 flags two
// consecutive calls to a / b and a % b use the quotient/remainder
// of just one call
void div8(uint8_t val);

// signed multiplications emulated
void imul8(uint8_t val);

void imul16(uint16_t val);

void imul16_bx(uint16_t val);
void imul16_dx(uint16_t val);

// mul16 this is needed for the rrola trick
void mul16(uint16_t val);

void swap_ax_dx();
void swap_bx_ax();

// a two-byte opcode that secretly accepts other values than 0a for the second byte
// making it possible to do a 8-bit div trick
void aam(uint16_t b);
#endif /* x86_h */
