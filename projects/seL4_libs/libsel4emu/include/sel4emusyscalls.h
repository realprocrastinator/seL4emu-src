#include <sel4/types.h>

/**
 * This the message structure for emulating the seL4 IPC, the mesassage has several types, using tag 
 * to distinguish them, if the mesassage uses the seL4_sys_emu tag, then the content emulates the registers
 * when the real seL4 syscall call happen. The registers order follows the syscall calling convention and is
 * architectual dependent. Here we use the x86_64 calling convention.   
 */



void seL4emu_DebugPutChar(char c);

void seL4emu_sys_send(seL4_Word sys, seL4_Word dest, seL4_Word info,
                  seL4_Word msg0, seL4_Word msg1, seL4_Word msg2, seL4_Word msg3);

void seL4emu_sys_reply(seL4_Word sys, seL4_Word info, seL4_Word msg0, 
                   seL4_Word msg1, seL4_Word msg2, seL4_Word msg3);

void seL4emu_sys_send_null(seL4_Word sys, seL4_Word dest, seL4_Word info);

void seL4emu_sys_recv(seL4_Word sys, seL4_Word src, seL4_Word *out_badge, seL4_Word *out_info,
                                seL4_Word *out_mr0, seL4_Word *out_mr1, seL4_Word *out_mr2, seL4_Word *out_mr3,
                                LIBSEL4_UNUSED seL4_Word reply);

void seL4emu_sys_send_recv(seL4_Word sys, seL4_Word dest, seL4_Word *out_dest, seL4_Word info,
                                     seL4_Word *out_info, seL4_Word *in_out_mr0, seL4_Word *in_out_mr1, seL4_Word *in_out_mr2, seL4_Word *in_out_mr3,
                                     LIBSEL4_UNUSED seL4_Word reply);

void seL4emu_sys_nbsend_recv(seL4_Word sys, seL4_Word dest, seL4_Word src, seL4_Word *out_dest,
                                       seL4_Word info, seL4_Word *out_info, seL4_Word *in_out_mr0, seL4_Word *in_out_mr1, seL4_Word *in_out_mr2,
                                       seL4_Word *in_out_mr3, seL4_Word reply);

void seL4emu_sys_null(seL4_Word sys);