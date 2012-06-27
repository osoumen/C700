
CC = wla-spc700
CFLAGS = -o
LD = wlalink
LDFLAGS = -vS

SFILES = VIOLIN.s
IFILES = defines.i
OFILES = VIOLIN.o

all: $(OFILES) makefile
	$(LD) $(LDFLAGS) linkfile result.rom

VIOLIN.o: VIOLIN.s defines.i
	$(CC) $(CFLAGS) VIOLIN.s

$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~ result.rom result.sym
