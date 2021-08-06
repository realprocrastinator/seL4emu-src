#include <stdlib.h>

// emulation
#include <emu/emu_globalstate.h>
#include <emu_arch/emu_boot.h>

// seL4
#include <arch/kernel/boot.h>
#include <arch/kernel/boot_sys.h>
#include <arch/kernel/vspace.h>
#include <kernel/boot.h>
#include <kernel/thread.h>
#include <machine.h>
#include <model/statedata.h>

void seL4emu_boot(void) {

  /* hellow world user image info */

  // ui_info_t ui_info;
  // ui_info.p_reg.start = 10563584;
  // ui_info.p_reg.end = 11759616;
  // ui_info.pv_offset = 6369280;

  boot_sys(MULTIBOOT_MAGIC, NULL);
}