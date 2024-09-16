#include "sys.h"

#include <stdint.h>
#include <stddef.h>
#include <c64.h>
#include <6502.h>

void nmi_handler() {
    __asm__("rti");
}

void sys_init() {
    __asm__("sei");

    // Kernal ROM off
    __asm__("lda #%b", ~0x07);
    __asm__("and 1");
    __asm__("ora #$05");
    __asm__("sta 1");

    // Disable CIA interrupts
    __asm__("lda #$7F");
    __asm__("sta %w+%b", (uint16_t) &CIA1, offsetof(struct __6526, icr));
    __asm__("sta %w+%b", (uint16_t) &CIA2, offsetof(struct __6526, icr));

    // Clear VIC interrupt flag
    __asm__("and %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, irr));
    __asm__("sta %w+%b", (uint16_t) &VIC, offsetof(struct __vic2, irr));

    // Acknowledge CIA interrupts
    __asm__("sta %w+%b", (uint16_t) &CIA1, offsetof(struct __6526, icr));
    __asm__("sta %w+%b", (uint16_t) &CIA2, offsetof(struct __6526, icr));

    // Stop the RESTORE key crashing the program
    __asm__("lda #<(%v)", nmi_handler);
    __asm__("sta $FFFA");
    __asm__("lda #>(%v)", nmi_handler);
    __asm__("sta $FFFB");

    // Use timer A for random numbers:
    // Leave bits 6 and 7
    //  0: Count clock ticks
    //  0: Don't load
    //  0: Continuous mode
    //  0: Don't care about the output
    //  0: No output
    //  1: Start timer
    CIA1.cra = CIA1.cra & 0x3F | 0x01;

    __asm__("cli");
}

uint_fast16_t sys_random_seed() {
    return ((uint_fast16_t) CIA1.ta_hi << 8) | CIA1.ta_lo;
}
