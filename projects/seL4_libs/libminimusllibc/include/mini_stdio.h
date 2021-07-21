/** 
 * This printing function will use the Linux write syscall to print to the console, 
 * this is not used for the seL4's debugputchar's emulation. 
 */
int mini_printf(const char *restrict fmt, ...);