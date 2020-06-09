//
//  x86.c
//  cocoamemories
//
//  Created by Virokannas, Simo on 6/8/20.
//  Copyright © 2020 IHIHI. All rights reserved.
//

#include "x86.h"

x86_reg r;

// implement some of the x86 functions in pseudo-register code

// div8 is usually provided by the compiler, with O1 flags two
// consecutive calls to a / b and a % b use the quotient/remainder
// of just one call
void div8(uint8_t val) {
    uint8_t qt, rm;
    qt=r.ax/val;
    rm=r.ax%val;
    r.al = qt; r.ah = rm;
}

// signed multiplications emulated
void imul8(uint8_t val) {
    r.ax = (int16_t)((int8_t)(r.al) * (int8_t)val);
}

void imul16(uint16_t val) {
    int32_t res = ((int32_t)((int16_t)(r.ax)) * (int16_t)val);
    r.ax = res & 0xffff;
    r.dx = (res >> 16) & 0xffff;
}

void imul16_bx(uint16_t val) {
    int32_t res = ((int32_t)((int16_t)(r.bx)) * (int16_t)val);
    r.bx = res & 0xffff;
    r.cf = (r.bx == res);
}
void imul16_dx(uint16_t val) {
    int32_t res = (int32_t)((int16_t)(r.dx) * (int16_t)val);
    r.dx = res & 0xffff;
    r.cf = (r.bx == res);
}

// mul16 this is needed for the rrola trick
void mul16(uint16_t val) {
    uint32_t res = (uint32_t)r.ax * (uint32_t)val;
    r.ax = res & 0xffff;
    res >>= 16;
    r.dx = res & 0xffff;
}

void swap_ax_dx() {
    r.swp=r.ax;r.ax=r.dx;r.dx=r.swp;
}
void swap_bx_ax() {
    r.swp=r.bx;r.bx=r.ax;r.ax=r.swp;
}

// a two-byte opcode that secretly accepts other values than 0a for the second byte
// making it possible to do a 8-bit div trick
void aam(uint16_t b) {
    uint8_t qt = r.al / b;
    uint8_t rm = r.al % b;
    r.al = rm; r.ah = qt;
}
void mov8(uint8_t *a, uint8_t *b) {
    *a = *b;
}

void mov16(uint16_t *a, uint16_t *b) {
    *a = *b;
}


