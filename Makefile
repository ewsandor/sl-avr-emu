sl_avr_emu : sl_avr_emu.o sl_avr_emu_tick.o
	cc -o sl_avr_emu sl_avr_emu.o sl_avr_emu_tick.o

sl_avr_emu.o : src/sl_avr_emu.cpp inc/sl_avr_emu.hpp inc/sl_avr_emu_tick.hpp inc/sl_avr_emu_types.hpp
	cc sl_avr_emu.o -c -Iinc/ src/sl_avr_emu.cpp

sl_avr_emu_tick.o : src/sl_avr_emu_tick.cpp inc/sl_avr_emu.hpp inc/sl_avr_emu_tick.hpp inc/sl_avr_emu_types.hpp inc/sl_avr_emu_bitops.hpp
	cc sl_avr_emu.o -c -Iinc/ src/sl_avr_emu_tick.cpp

clean :
	rm *.o sl_avr_emu
