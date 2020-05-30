//
//  mainloop.c
//  cocoamemories
//
//  An attempt at replicating "memories" by Desire
//  https://www.pouet.net/prod.php?which=85227
//
//  All original code and the breakdown assembly comments
//  kindly provided by HellMood at http://www.sizecoding.org/wiki/Memories
//

#include "mainloop.h"


// A simple registry structure to get access to the 8/16 bit registers in tandem
struct {
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
} r;

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

// scrolling tilted plane
void fx0() {
    r.ax = 0x1329;  // mov ax,0x1329    ; initialize with constant
    r.dh += r.al;   // add dh,al        ; preventing divide overflow
    div8(r.dh);     // div dh           ; reverse divide AL = C/Y'
    swap_ax_dx();   // xchg dx,ax       ; DL = C/Y', AL = X
    imul8(r.dl);    // imul dl          ; AH = CX/Y'
    r.dx -= r.bp;   // sub dx,bp        ; DL = C/Y'-T
    r.ah ^= r.dl;   // xor ah,dl        ; AH = (CX/Y') ^ (C/Y'-T)
    r.al = r.ah;    // mov al,ah        ; move to AL
    r.al &= 4+8+16; // and al,4+8+16    ; select special pattern
}

// zooming circles
void fx1() {
    // mov al,dh        ; get Y in AL
    r.al = r.dh;
    // sub al,100        ; align Y vertically
    r.al -= 100;
    // imul al            ; AL = Y²
    imul8(r.al);
    // xchg dx,ax        ; Y²/256 in DH, X in AL
    swap_ax_dx();
    // imul al            ; AL = X²
    imul8(r.al);
    // add dh,ah        ; DH = (X² + Y²)/256
    r.dh += r.ah;
    // mov al,dh        ; AL = (X² + Y²)/256
    r.al = r.dh;
    // add ax,bp        ; offset color by time
    r.ax += r.bp;
    // and al,8+16        ; select special rings
    r.al &= 8 | 16;
}

// array of chessboards
void fx2() {
    // xchg dx,ax        ; get XY into AX
    swap_ax_dx();
    // sub ax,bp        ; subtract time from row
    r.ax -= r.bp;
    // xor al,ah        ; XOR pattern (x xor y)
    r.al = r.al ^ r.ah;
    // or al,0xDB        ; pattern for array of boards
    r.al |= 0xdb;
    // add al,13h        ; shift to good palette spot
    r.al += 0x13;
}

// parallax checkerboards
void fx3() {
    
    // mov cx,bp        ; set inital point to time
    r.cx = r.bp;
    // mov bx,-16        ; limit to 16 iterations
    r.bx = -16;
fx3L:
    // add cx,di        ; offset point by screenpointer
    r.cx += r.di;
    // mov ax,819        ; magic, related to Rrrola constant
    r.ax = 819;
    // imul cx            ; get X',Y' in DX
    imul16(r.cx);
    // ror dx,1        ; set carry flag on "hit"
    uint8_t msb = r.dx & 0x01;
    r.dx = ((r.dx & 0x01 << 15)) | (r.dx >> 1);
    // inc bx            ; increment iteration count
    r.bx += 1;
    // ja fx3L            ; loop until "hit" or "iter=max"
    if(msb==0 && r.bx != 0) goto fx3L;
    // lea ax,[bx+31]    ; map value to standard gray scale
    r.ax = r.bx + 31;
}

// sierpinski rotozoomer
void fx4() {
    // lea cx,[bp-2048]; center time to pass zero
    r.cx = r.bp-2048;
    // sal cx,3        ; speed up by factor 8!
    r.cx <<= 3;
    // movzx ax,dh        ; get X into AL
    r.ax = r.dh;
    // movsx dx,dl        ; get Y int DL
    r.dx = (int8_t)r.dl;
    // mov bx,ax        ; save X in BX
    r.bx = r.ax;
    // imul bx,cx        ; BX = X*T
    imul16_bx(r.cx);
    // add bh,dl        ; BH = X*T/256+Y
    r.bh += r.dl;
    // imul dx,cx        ; DX = Y*T
    imul16_dx(r.cx);
    // sub al,dh        ; AL = X-Y*T/256
    r.al -= r.dh;
    // and al,bh        ; AL = (X-Y*T/256)&(X*T/256+Y)
    r.al &= r.bh;
    // and al,252        ; thicker sierpinski
    r.al &= 252;
    // salc            ; set pixel value to black
    // jnz fx4q        ; leave black if not sierpinski
    // mov al,0x2A        ; otherwise: a nice orange
    r.al = r.al?0x00:0x2a;
}

// raycast bent tunnel
void fx5() {
    // fake push stack
    uint16_t tmpdx = r.dx;

    // mov cl,-9        ; start with depth 9 (moves backwards)
    r.cl = 0xf7;
fx5L:
    r.dx = tmpdx;
    // push dx            ; save DX, destroyed inside the loop
    // mov al,dh    ; Get Y into AL
    r.al = r.dh;
    // sub al,100    ; Centering Y has to be done "manually".
    r.al -= 100;
    // imul cl        ; Multiply AL=Y by the current distance, to get a projection(1)
    imul8(r.cl);
    // xchg ax,dx    ; Get X into AL, while saving the result in DX (DH)
    swap_ax_dx();
    // add al,cl    ; add distance to projection, (bend to the right)
    r.al += r.cl;
    // imul cl        ; Multiply AL=X by the current distance, to get a projection(2)
    imul8(r.cl);
    // mov al,dh    ; Get projection(1) in AL
    r.al = r.dh;
    // xor al,ah    ; combine with projection(2)
    r.al ^= r.ah;
    // add al,4    ; center the walls around 0
    r.al += 4;
    // test al,-8    ; check if the wall is hit
    // pop dx            ; restore DX
    // loopz fx5L        ; repeat until "hit" or "iter=max"
    r.cx--;
    if((r.al & 0xf8)==0 && r.cx > 0) goto fx5L;
    // sub cx,bp        ; offset depth by time
    r.cx -= r.bp;
    // xor al,cl        ; XOR pattern for texture
    r.al ^= r.cl;
    // aam 6            ; irregular pattern with MOD 6
    aam(6);
    // add al,20        ; offset into grayscale palette
    r.al += 20;
}

// ocean night to day
void fx6() {
    // sub dh,120            ; check if pixel is in the sky
    // js fx6q                ; quit if that's the case
    if(r.dh<120) return;

    // set up memory pointers
    int16_t *m1 = (int16_t *)(r.mem + r.bx + r.si);
    int32_t *m2 = (int32_t *)(r.mem + r.bx + r.si);
    int32_t *m3 = (int32_t *)(r.mem + r.bx + r.si - 1);
    // mov [bx+si],dx        ; move XY to a memory location
    *m1 = r.dx;
    // fild word [bx+si]    ; read memory location as integer
    r.st0 = *m1;
    // fidivr dword [bx+si]; reverse divide by constant
    r.st0 = (*m2) / r.st0;
    // fstp dword [bx+si-1]; store result as floating point
    *m3 = (*(int32_t *)(&r.st0));
    // mov ax,[bx+si]        ; get the result into AX
    r.ax = (uint32_t)(*m1);
    // add ax,bp            ; modify color by time
    r.ax += r.bp;
    // and al,128            ; threshold into two bands
    r.al &= 128;
    // dec ax                ; beautify colors to blue/black
    r.ax --;
}

void dummy() {
    // just a black screen
    r.ax = 0;
    return;
}

void stop() {
    exit(0);
}

// effects table
void (*tb[])(void)= {fx2,fx1,fx0,fx3,fx4,fx5,fx6,dummy,stop};

void advance(unsigned char *a000, unsigned short bp) {
    // inject our fake time pointer
    r.bp = bp;
    r.di = 0;
    do {
        r.ax = 0xcccd;
        // mul di                    ; transform screen pointer to X, Y
        mul16(r.di);

        // add al,ah                ; use transformation garbage as
        r.al += r.ah;
        // xor ah,ah                ; pseudorandom value and clear AH
        r.ah ^= r.ah;
        // add ax,bp                ; add time value to random value
        r.ax += bp;
        // shr ax,9                ; divide by 512 (basically the speed)
        r.ax >>= 9;
        // and al,15                ; filter effect number
        r.al &= 15;
        // xchg bx,ax                ; move effect number to BX
        swap_bx_ax();
        // mov bh,1                ; reset BH to align with start of code
        r.bh = 1;
        // call bx                    ; call the effect
        (*tb[r.bl])();
        // stosb                    ; write the return value and advance
        a000[r.di++ - 160] = r.al;
        // inc di                    ; triple interlace trick for after
        // inc di                    ; effect and smoothing the animation
        r.di+=2;
    } while (r.di != 0);
}
