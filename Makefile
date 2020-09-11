CFLAGS = -g
LDFLAGS = -g

sl_avr_emu : sl_avr_emu.o sl_avr_emu_hex.o sl_avr_emu_interrupt.o sl_avr_emu_tick.o sl_avr_emu_timer.o
	cc $(LDFLAGS) -o sl_avr_emu sl_avr_emu.o sl_avr_emu_hex.o sl_avr_emu_interrupt.o sl_avr_emu_tick.o sl_avr_emu_timer.o -lpthread

sl_avr_emu.o : src/sl_avr_emu.c inc/sl_avr_emu.h inc/sl_avr_emu_interrupt.h inc/sl_avr_emu_tick.h inc/sl_avr_emu_types.h inc/sl_avr_emu_timer.h
	cc $(CFLAGS) -c -Iinc/ src/sl_avr_emu.c

sl_avr_emu_hex.o : src/sl_avr_emu_hex.c inc/sl_avr_emu.h inc/sl_avr_emu_hex.h inc/sl_avr_emu_types.h
	cc $(CFLAGS) -c -Iinc/ src/sl_avr_emu_hex.c

sl_avr_emu_interrupt.o : src/sl_avr_emu_interrupt.c inc/sl_avr_emu.h inc/sl_avr_emu_interrupt.h inc/sl_avr_emu_tick.h inc/sl_avr_emu_types.h inc/sl_avr_emu_timer.h inc/sl_avr_emu_bitops.h
	cc $(CFLAGS) -c -Iinc/ src/sl_avr_emu_interrupt.c

sl_avr_emu_tick.o : src/sl_avr_emu_tick.c inc/sl_avr_emu.h inc/sl_avr_emu_interrupt.h inc/sl_avr_emu_tick.h inc/sl_avr_emu_types.h inc/sl_avr_emu_timer.h inc/sl_avr_emu_bitops.h
	cc $(CFLAGS) -c -Iinc/ src/sl_avr_emu_tick.c

sl_avr_emu_timer.o : src/sl_avr_emu_timer.c inc/sl_avr_emu.h inc/sl_avr_emu_types.h inc/sl_avr_emu_bitops.h inc/sl_avr_emu_timer.h
	cc $(CFLAGS) -c -Iinc/ src/sl_avr_emu_timer.c

clean :
	rm *.o sl_avr_emu
