#include "fractal_myflpt.h"
#include "swap.h"
#include "vga.h"
#include "cache.h"
#include "perf.h"
#include <stddef.h>
#include <stdio.h>

// Constants describing the output device
#define SCREEN_WIDTH  512   //!< screen width
#define SCREEN_HEIGHT 512   //!< screen height

//! \brief Frame buffer, deliberately at file scope so that it is allocated in .bss just
//!        above the code. Declared inside main() it becomes a 512 kiB variable-length
//!        array carved out of the stack, which places it against the very top of the
//!        SDRAM, ending 8 bytes below main's saved return address.
static rgb565 frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

//! \brief Report an illegal instruction instead of continuing silently.
//! \note  Overrides the __weak default in support/src/exception.c.
void illegal_instruction_handler(void) {
   printf("ILL @ EPCR=0x%08X insn=0x%08X\n", SPR_READ(32), SPR_READ(80));
   while (1) {}
}

const uint16_t N_MAX = 64;    //!< maximum number of iterations

int main() {

   // Constants describing the initial view port on the fractal function
   const myflpt FRAC_WIDTH = float_to_myflpt(3.0f); //!< default fractal width (3.0 in myflpt
   const myflpt CX_0 = float_to_myflpt(-2.0f);      //!< default start x-coordinate (-2.0 in myflpt
   const myflpt CY_0 = float_to_myflpt(-1.5f);      //!< default start y-coordinate (-1.5 in myflpt

   volatile unsigned int *vga = (unsigned int *) 0x50000020;
   volatile unsigned int reg, hi;
   myflpt delta = float_to_myflpt(3.0f / 512.0f);

   int i;
   vga_clear();
   printf("Starting drawing a fractal\n");

#ifdef __OR1300__   
   /* enable the caches */
   // optimize here later.
   icache_write_cfg( CACHE_DIRECT_MAPPED | CACHE_SIZE_8K | CACHE_REPLACE_FIFO );
   dcache_write_cfg( CACHE_FOUR_WAY | CACHE_SIZE_8K | CACHE_REPLACE_LRU | CACHE_WRITE_BACK );
   icache_enable(1);
   dcache_enable(1);
#endif
   /* Enable the vga-controller's graphic mode */
   vga[0] = swap_u32(SCREEN_WIDTH);
   vga[1] = swap_u32(SCREEN_HEIGHT);
   vga[2] = swap_u32(1);
   vga[3] = swap_u32((unsigned int)&frameBuffer[0]);
   /* Clear screen */
   for (i = 0 ; i < SCREEN_WIDTH*SCREEN_HEIGHT ; i++) frameBuffer[i]=0;

   /* Configure the profiling counters. The count masks may only be written
    * while the profiling module is disabled, hence the perf_stop() first.
    * See virtualprototype/modules/or1300/doc/profiling_module.txt */
   perf_init();
   perf_stop();
   perf_set_mask(PERF_COUNTER_0, PERF_STALL_CYCLES_MASK);
   perf_set_mask(PERF_COUNTER_1, PERF_BUS_IDLE_MASK);
   perf_set_mask(PERF_COUNTER_2, PERF_ICACHE_MISS_MASK);
   perf_set_mask(PERF_COUNTER_3, PERF_EXECUTED_INSTRUCTIONS_MASK);
   perf_set_mask(PERF_COUNTER_4, PERF_DCACHE_MISS_MASK);
   /* Counted separately from PERF_STALL_CYCLES_MASK, and on this program it is the
    * dominant cost, so it has to be measured rather than inferred. */
   perf_set_mask(PERF_COUNTER_5, PERF_ICACHE_MISS_PENALY_MASK);

   perf_start();
   draw_fractal(frameBuffer,SCREEN_WIDTH,SCREEN_HEIGHT,&calc_mandelbrot_point_soft, &iter_to_colour,CX_0,CY_0,delta,N_MAX);
   perf_stop();

   /* The profiling module is pipelined: wait at least six cycles after
    * stopping it before reading the counters out. */
   for (volatile int d = 0; d < 16; ++d);

   perf_print_time  (PERF_COUNTER_RUNTIME, "run time");
   perf_print_cycles(PERF_COUNTER_RUNTIME, "run time");
   perf_print_cycles(PERF_COUNTER_0,       "cpu stall cycles");
   perf_print_cycles(PERF_COUNTER_1,       "bus idle cycles");
   perf_print_cycles(PERF_COUNTER_2,       "i-cache misses");
   perf_print_cycles(PERF_COUNTER_3,       "executed instructions");
   perf_print_cycles(PERF_COUNTER_4,       "d-cache misses");
   perf_print_cycles(PERF_COUNTER_5,       "i-cache miss penalty");
#ifdef __OR1300__
   dcache_flush();
#endif
   printf("Done\n");
}
