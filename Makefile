# Makefile to build librfm

MCU = atmega328p
F_CPU = 8000000

MAIN = librfm.c

CC = avr-gcc

CFLAGS =  -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL
CFLAGS += -O2 -I.
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -g -ggdb
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -mrelax
CFLAGS += -std=gnu99
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
# CFLAGS += --param=min-pagesize=0

TARGET = $(strip $(basename $(MAIN)))
SRC += $(TARGET).c

OBJ = $(SRC:.c=.o) 
OBJ = $(SRC:.S=.o)

$(TARGET).elf: librfm.h pins.h spi.h types.h utils.h Makefile

all: $(TARGET).elf

%.elf: $(SRC)
	$(CC) $(CFLAGS) $(SRC) --output $@ 

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).obj \
	$(TARGET).o $(TARGET).d $(TARGET).eep $(TARGET).lst \
	$(TARGET).lss $(TARGET).sym $(TARGET).map $(TARGET)~ \
	$(TARGET).eeprom
