TEMPLATEROOT = ../Fall2018-Lab

# compilation flags for gdb

CFLAGS  = -O0 -g
ASFLAGS = -g stm32f30x_

# object files

OBJS=  $(STARTUP) 	main.o \
			draw.o \
			f3d_user_btn.o \
			f3d_led.o \
			f3d_uart.o \
			f3d_lcd_sd.o \
			glcdfont.o \
			stm32f30x_rcc.o \
			stm32f30x_gpio.o \
			stm32f30x_usart.o \
			newlib_stubs.o \
			stm32f30x_spi.o \
			stm32f30x_tim.o \
			f3d_delay.o \
			f3d_accel.o \
			f3d_i2c.o \
			stm32f30x_i2c.o \
			pokemon.o
# include common make file

include $(TEMPLATEROOT)/c335.mk
