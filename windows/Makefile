PROGRAM = clip2qr.exe

LIBS    = -mwindows

QRLIB_PATH = ..\QR-Code-generator\c

CLIP2QR_C = clip2qr-windows.c $(QRLIB_PATH)\qrcodegen.c
CLIP2QR_H = $(QRLIB_PATH)\qrcodegen.h resource.h

CFLAGS = -std=c99 -I$(QRLIB_PATH)

all: $(PROGRAM)

$(PROGRAM): $(OBJS) Makefile clip2qr.res
	gcc $(CFLAGS) -o $(PROGRAM) $(CLIP2QR_C) clip2qr.res $(LIBS)
	strip $(PROGRAM)

%.res : %.rc $(HDRS) clip2qr.ico
	windres --use-temp-file -O coff $< $@

clean:
	rm -f $(PROGRAM)
	rm -f *.res
