#include "sys_init.h"

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

    __asm__("cli");
}
