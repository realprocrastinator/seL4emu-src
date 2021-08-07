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
#include <arch/kernel/multiboot.h>

extern char *ki_boot_end;
extern char *ki_end;
extern char *ki_skim_start;
extern char *ki_skim_end;

void seL4emu_boot(void) {

  /* map a file backed memory to emulate the physical memory */

  // FAKE for testing
  /* fake the kernel image virtual address */
  ki_boot_end = (void *)0xffffffff80800000;
  ki_end = (void *)0xffffffff80a13000;
  ki_skim_start = (void *)0xffffffff80800000;
  ki_skim_end = (void *)0xffffffff80a00000;

  /* hellow world user image info */

  // ui_info_t ui_info;
  // ui_info.p_reg.start = 10563584;
  // ui_info.p_reg.end = 11759616;
  // ui_info.pv_offset = 6369280;

  /* User input for the multiboot info, we fake this, could be nicer if we parse
  the real image to obtain the info */
  /* HARD CODED! */
  multiboot_info_t mbi = {0};
  mbi.part1.flags |= 591;
  mbi.part1.mod_count = 1;
  mbi.part1.mem_lower = 639;
  mbi.part1.mem_upper = 523136;
  mbi.part1.boot_device = 2147549183;
  mbi.part1.cmdline = 10563637;

  mbi.part2.mmap_length = 144;

  boot_sys(MULTIBOOT_MAGIC, &mbi);
}