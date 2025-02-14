#include "kernel.h"
#include "common.h"

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

void putchar(char ch) { sbi_call(ch, 0, 0, 0, 0, 0, 0, 0x1); }

int getchar(void) { return sbi_call(0, 0, 0, 0, 0, 0, 0, 0x02).error; }

void puts(const char *s) {
    while (*s) {
        putchar(*s++);
    }
}

void get_hart_status() {
    puts("Enter hart ID: ");
    char hart_id = getchar() - '0';
    putchar(hart_id + '0');
    putchar('\n');

    struct sbiret ret = sbi_call(hart_id, 0, 0, 0, 0, 0, 0, 0x48534D);

    puts("Hart status: ");
    putchar(ret.value + '0');
    putchar('\n');
}

extern char __bss[], __bss_end[], __stack_top[];

void kernel_main(void) {
    puts("\nHi there!) It's an interactive menu!\n");
    puts("Chose some OpenSBI commands:\n");
    puts("1. Get SBI implementation version\n");
    puts("2. Hart get status\n");
    puts("3. Hart stop\n");
    puts("4. System Shutdown\n");

    while (1) {
        puts("\n");
        int input;
        while ((input = getchar()) == -1)
            ;
        puts("Chosen option: ");
        putchar(input);
        puts("\n");

        switch (input) {
        case '1': {
            struct sbiret result = sbi_call(0, 0, 0, 0, 0, 0, 2, 0x10);
            int major = (result.value >> 16) & 0xFFFF;
            int minor = result.value & 0xFFFF;

            printf("SBI implementation version: %d.%d\n", major, minor);
            break;
        }
        case '2': {
            puts("Enter hart ID: ");
            int hart_id;
            while ((hart_id = getchar()) == -1)
                ;
            putchar(hart_id);
            puts("\n");

            struct sbiret res = sbi_call(hart_id, 0, 0, 0, 0, 0, 2, 0x48534D);

            puts("Hart status: ");
            putchar(res.value + '0');
            puts("");
            break;
        }
        case '3': {
            puts("Stopping hart...");
            sbi_call(0, 0, 0, 0, 0, 0, 1, 0x48534D);
            break;
        }
        case '4': {
            puts("Shutting the system down...");
            sbi_call(0, 0, 0, 0, 0, 0, 0, 0x08);
            break;
        }
        default:
            puts("\nplease type in the variant :)\n");
        }

        puts("\ngive me another one, i'm hungry:");
    }
}

__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // Устанавливаем указатель стека
        "j kernel_main\n" // Переходим к функции main ядра
        :
        : [stack_top] "r"(
            __stack_top) // Передаём верхний адрес стека в виде %[stack_top]
    );
}
