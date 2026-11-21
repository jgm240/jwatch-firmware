/* AppSandbox_Invoke(void* entry, void* newSp): the one piece of this feature that has to run
 * as hand-written assembly rather than C, because it has to move the stack pointer itself -
 * something no C function can safely do to its own currently-executing frame.
 *
 * r0 = entry (the sandboxed blob's first instruction), r1 = newSp (top of its stack, inside
 * the slot). See AppSandbox.cpp/.h for why: this firmware's FreeRTOS port keeps task code on
 * PSP (confirmed from port.c's vPortSVCHandler), so "the stack pointer" here really is PSP,
 * and exception/interrupt entry still lands safely on the untouched MSP regardless of what
 * this does to PSP - as long as interrupts stay masked for the whole window, since FreeRTOS
 * must never try to preempt a task mid-way through this with PSP pointing somewhere it
 * doesn't recognize.
 *
 * Order matters here in one place that isn't obvious: AppSandbox_DisableMpu() is called
 * *before* restoring the real stack pointer, while sp still points into the slot - that's
 * deliberate. The slot is one of the regions the MPU allows while it's armed, so a normal
 * function call still works there; the real task stack is not, so switching back to it before
 * the MPU is actually turned off would fault on that call's own prologue.
 */
    .syntax unified
    .thumb

    .section .text.AppSandbox_Invoke
    .global AppSandbox_Invoke
    .type AppSandbox_Invoke, %function
    .thumb_func
AppSandbox_Invoke:
    push    {r4, r5, r6, lr}
    mov     r5, r0                  @ save entry (r0/r1 are caller-saved - would not survive the bl below)
    mov     r6, r1                  @ save newSp
    cpsid   i                       @ mask interrupts for the whole sandboxed window
    bl      AppSandbox_EnableMpu    @ still on the real stack here - fine, MPU isn't armed yet
    mov     r4, sp                  @ save the real (post-prologue) stack pointer
    orr     r5, r5, #1              @ force the Thumb bit on the entry address
    mov     sp, r6                  @ switch onto the app's own stack, inside the slot
    blx     r5                      @ run the sandboxed app; a fault here resets instead of returning
    bl      AppSandbox_DisableMpu   @ called while sp is still inside the slot - see note above
    mov     sp, r4                  @ only now safe to restore the real stack pointer
    cpsie   i
    pop     {r4, r5, r6, pc}
    .size AppSandbox_Invoke, . - AppSandbox_Invoke
