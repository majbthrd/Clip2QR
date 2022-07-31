CFLAGS  = -fstack-protector -fstack-protector-all
CFLAGS += $(shell fltk-config --cxxflags)
CFLAGS += -O3

# choose your linking option: either expect the target to have the libraries or statically link them into the executable
LFLAGS += $(shell fltk-config --ldflags)
#LFLAGS += $(shell fltk-config --ldstaticflags)

QRLIB_PATH = ../QR-Code-generator/c

CLIP2QR_C = clip2qr-fltk.cpp $(QRLIB_PATH)/qrcodegen.c
CLIP2QR_H = $(QRLIB_PATH)/qrcodegen.h

CFLAGS += -I$(QRLIB_PATH)

all: clip2qr

clip2qr: Makefile $(CLIP2QR_C) $(CLIP2QR_H)
	g++ $(CFLAGS) $(CLIP2QR_C) -o $@ $(LFLAGS)
	strip $@

clean:
	rm -f clip2qr
