
#include <stdio.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <limits.h>
#include "8250_uart.h"

#define SYS_write 64

#undef strcmp

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

uart8250_t * uart = (uart8250_t *) (0x10000000) ;

static uintptr_t syscall(uintptr_t which, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
  volatile uint64_t magic_mem[8] __attribute__((aligned(64)));
  magic_mem[0] = which;
  magic_mem[1] = arg0;
  magic_mem[2] = arg1;
  magic_mem[3] = arg2;
  __sync_synchronize();

  tohost = (uintptr_t)magic_mem;
  while (fromhost == 0)
    ;
  fromhost = 0;

  __sync_synchronize();
  return magic_mem[0];
}

// void* memset(void* dest, int byte, size_t len)
// {
//   if ((((uintptr_t)dest | len) & (sizeof(uintptr_t)-1)) == 0) {
//     uintptr_t word = byte & 0xFF;
//     word |= word << 8;
//     word |= word << 16;
//     word |= word << 16 << 16;

//     uintptr_t *d = dest;
//     while ((uintptr_t)d < (uintptr_t)(dest + len))
//       *d++ = word;
//   } else {
//     char *d = dest;
//     while (d < (char*)(dest + len))
//       *d++ = byte;
//   }
//   return dest;
// }


void* memcpy(void* dest, const void* src, size_t len)
{
  if ((((uintptr_t)dest | (uintptr_t)src | len) & (sizeof(uintptr_t)-1)) == 0) {
    const uintptr_t* s = src;
    uintptr_t *d = dest;
    while (d < (uintptr_t*)(dest + len))
      *d++ = *s++;
  } else {
    const char* s = src;
    char *d = dest;
    while (d < (char*)(dest + len))
      *d++ = *s++;
  }
  return dest;
}

int _read(int file, char *ptr, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        //ptr[i] = uart8250_getc();
    }

    return len;
}

int _write(int file, char *ptr, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        if (ptr[i] == '\n')
        {
            uart_putc(uart, '\r');
        }
        uart_putc(uart, ptr[i]);
    }

    return len;
}

int _lseek(int file, int ptr, int dir)
{
    //errno = ESPIPE;
    return -1;
}

int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    //errno = ENOTTY;
    return 0;
}

void* _sbrk(int increment)
{
    extern char _end; // From linker script
    static char* heap_end = &_end;

    char* current_heap_end = heap_end;
    heap_end += increment;

    return current_heap_end;
}

void __attribute__((noreturn)) tohost_exit(uintptr_t code)
{
  tohost = (code << 1) | 1;
  while (1);
}

void _exit(int code)
{
    tohost_exit(code);
}

void _abort()
{
  exit(128 + SIGABRT);
}

int _getpid(void)
{
  return 1;
}

int _kill(int pid, int sig)
{
    //errno = EINVAL;
    return -1;
}

void _init(){
    uart_init(uart);
}