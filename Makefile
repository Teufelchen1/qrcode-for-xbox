XBE_TITLE = qr-code-for-xbox
GEN_XISO = $(XBE_TITLE).iso

# Adjust this location to your system!
NXDK_DIR = /Users/$(USER)/nxdk
SOURCES = main.c qrcodegen.c
SRCS += $(SOURCES)
NXDK_SDL = y
include $(NXDK_DIR)/Makefile

macOS: objects
	$(CC) -o main.out main.o qrcodegen.o -L/usr/local/lib -lSDL2

objects :
	$(CC) $(CFLAGS) -I/usr/local/include/SDL2 -c $(SOURCES)
