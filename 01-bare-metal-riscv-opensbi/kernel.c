/*
 * Bare-metal RISC-V (rv32) "kernel" running on top of OpenSBI in QEMU.
 *
 * No OS, no libc (-ffreestanding -nostdlib). We bring up our own stack in the
 * naked boot stub, then talk to the SBI firmware (M-mode) from S-mode through
 * the `ecall` instruction. The SBI calling convention passes arguments in
 * a0..a5, the function id (FID) in a6 and the extension id (EID) in a7, and
 * returns {error, value} in {a0, a1}.
 */
#include "common.h"
#include "kernel.h"

/* --- SBI extension IDs (EID) and function IDs (FID) used below ----------- */
#define SBI_EXT_LEGACY_PUTCHAR 0x01     /* legacy console putchar           */
#define SBI_EXT_LEGACY_GETCHAR 0x02     /* legacy console getchar           */
#define SBI_EXT_LEGACY_SHUTDOWN 0x08    /* legacy system shutdown           */
#define SBI_EXT_BASE 0x10               /* base extension                   */
#define SBI_FID_BASE_IMPL_VERSION 2     /*   get SBI implementation version */
#define SBI_EXT_HSM 0x48534D            /* "HSM" — hart state management     */
#define SBI_FID_HSM_HART_STOP 1         /*   stop the calling hart          */
#define SBI_FID_HSM_HART_STATUS 2       /*   get hart status                */

static inline struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3,
                                     long arg4, long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "+r"(a0), "+r"(a1)
                         : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                         : "memory");

    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, SBI_EXT_LEGACY_PUTCHAR);
}

/* Legacy console getchar returns the character (or -1) in a0 == .error. */
int getchar(void) {
    return sbi_call(0, 0, 0, 0, 0, 0, 0, SBI_EXT_LEGACY_GETCHAR).error;
}

void puts(const char *s) {
    while (*s)
        putchar(*s++);
}

/* Blocking read of a single character from the SBI console. */
static int getchar_blocking(void) {
    int c;
    while ((c = getchar()) == -1)
        ;
    return c;
}

extern char __bss[], __bss_end[], __stack_top[];

void kernel_main(void) {
    puts("\nHi there!) It's an interactive menu!\n");
    puts("Choose an OpenSBI command:\n");
    puts("1. Get SBI implementation version\n");
    puts("2. Hart get status\n");
    puts("3. Hart stop\n");
    puts("4. System shutdown\n");

    while (1) {
        puts("\n");
        int input = getchar_blocking();
        puts("Chosen option: ");
        putchar(input);
        puts("\n");

        switch (input) {
        case '1': {
            struct sbiret r = sbi_call(0, 0, 0, 0, 0, 0,
                                       SBI_FID_BASE_IMPL_VERSION, SBI_EXT_BASE);
            int major = (r.value >> 16) & 0xFFFF;
            int minor = r.value & 0xFFFF;
            printf("SBI implementation version: %d.%d\n", major, minor);
            break;
        }
        case '2': {
            puts("Enter hart ID: ");
            int hart_id = getchar_blocking();
            putchar(hart_id);
            puts("\n");
            struct sbiret r = sbi_call(hart_id - '0', 0, 0, 0, 0, 0,
                                       SBI_FID_HSM_HART_STATUS, SBI_EXT_HSM);
            puts("Hart status: ");
            putchar(r.value + '0');
            puts("\n");
            break;
        }
        case '3':
            puts("Stopping hart...");
            sbi_call(0, 0, 0, 0, 0, 0, SBI_FID_HSM_HART_STOP, SBI_EXT_HSM);
            break;
        case '4':
            puts("Shutting the system down...");
            sbi_call(0, 0, 0, 0, 0, 0, 0, SBI_EXT_LEGACY_SHUTDOWN);
            break;
        default:
            puts("\nplease type in a valid option :)\n");
        }

        puts("\ngive me another one, i'm hungry:");
    }
}

__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" /* set up the stack pointer            */
        "j kernel_main\n"       /* jump into the C entry point         */
        :
        : [stack_top] "r"(__stack_top));
}
