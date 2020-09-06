CFLAGS=-g
LDFLAGS=-g

sl_avr_emu : sl_avr_emu.o sl_avr_emu_tick.o sl_avr_emu_hex.o
	cc -o sl_avr_emu sl_avr_emu.o sl_avr_emu_tick.o sl_avr_emu_hex.o

sl_avr_emu.o : src/sl_avr_emu.c inc/sl_avr_emu.h inc/sl_avr_emu_tick.h inc/sl_avr_emu_types.h
	cc -c -Iinc/ src/sl_avr_emu.c

sl_avr_emu_tick.o : src/sl_avr_emu_tick.c inc/sl_avr_emu.h inc/sl_avr_emu_tick.h inc/sl_avr_emu_types.h inc/sl_avr_emu_bitops.h
	cc -c -Iinc/ src/sl_avr_emu_tick.c

sl_avr_emu_hex.o : src/sl_avr_emu_hex.c inc/sl_avr_emu.h inc/sl_avr_emu_hex.h inc/sl_avr_emu_types.h
	cc -c -Iinc/ src/sl_avr_emu_hex.c

clean :
	rm *.o sl_avr_emu
