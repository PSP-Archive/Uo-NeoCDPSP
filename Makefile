TARGET = neocd
INCDIR = src src/psp src/zip src/68k src/z80 src/video src/libmad

#ZLIB only works well with O3

CFLAGS= -O3 -Wall -fstrict-aliasing -ffast-math \
	-DINLINE="static inline" -DLOWERCASEFILES \
	 -include src/psp/floatonly.h \
	-DCPU68K_USE_C68K \
	-D_PSPyo_ -DZ80_EMULATED  -DSOUND -DENABLE_CDDA -DUSE_PD4990A\
	-DUSE_MEMCARD -DFPM_MIPS -DPSP_MODE -DVIDEO_PSP
#	-DUSE_CZ80 -DCPU68K_USE_MUSASHI -DCPU68K_USE_C68K
#	-DCPU68K_USE_C68K  -funsigned-char -DUSE_CZ80 -DUSE_MAMEZ80

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
USE_PSPSDK_LIBC=1
LIBS = -lm -lpspaudio -lpspgu -lpsppower -lz
#-lm -lc

EXTRA_TARGETS = mkdir EBOOT.PBP
EXTRA_CLEAN = pspclean
PSP_EBOOT_TITLE = "NeoCDPSP v"`grep VERSION_STR src/psp/psp.h|sed 's/.*"\(.*\)"/\1/'`
PSP_EBOOT_ICON = data/icon0.png
PSP_EBOOT_PIC1 = data/pic1.png
PSP_EBOOT_SND0 = data/snd0.at3

# Object files
MAINOBJS = obj/neocd.o obj/pd4990a.o obj/memcard.o \
	obj/zip/zfile.o obj/zip/unzip.o obj/ingame.o

EXTOBJS=  obj/psp/pg.o obj/psp/psplib.o obj/psp/filer.o obj/psp/psp.o \
	obj/psp/imageio.o obj/psp/menu.o obj/psp/blitter.o

NCDLIBS = obj/68k.a obj/z80.a obj/memory.a obj/cdrom.a obj/video.a obj/cdaudio.a \
	obj/input.a obj/sound.a obj/libmad.a

OBJS= $(MAINOBJS) $(EXTOBJS) $(NCDLIBS)

MKDIR= @mkdir -p


#neocd$(EXE):	$(OBJS)
#		$(LD) $(LDFLAGS) $(CFLAGS) $(MAINOBJS) $(EXTOBJS) $(LIBS) $(EXTLIBS) -o $@
#	./outpatch $@ neocd.psp "USERPROG"
#	mipsel-elf-strip neocd.psp
#	./elf2pbp neocd.psp "NEOCD PSP 0.1"
#	cp EBOOT.PBP $(PSPE_DIR)/ms0/PSP/GAME/NEOCD

# Rules to make libraries


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

obj/68k.a:	obj/68k/c68k/c68kexec.o	obj/68k/c68k/c68k.o obj/68k/c68k/cpu68k.o
#obj/68k.a:	obj/68k/c68k/m68kops.o \
#		obj/68k/c68k/m68kcpu.o \
#		obj/68k/c68k/m68kopac.o \
#		obj/68k/c68k/m68kopdm.o \
#		obj/68k/c68k/m68kopnz.o \
#		obj/68k/c68k/m68kdasm.o obj/68k/c68k/cpu68k.o
#obj/z80.a:	obj/z80/mamez80/z80.o obj/z80/z80intrf.o
#obj/z80.a:	obj/z80/cz80/cz80.o obj/z80/z80intrf.o
obj/z80.a:	obj/z80/z80intrf.o obj/z80/z80.o
obj/memory.a:	obj/memory/c68k/memory.o
obj/cdrom.a:	obj/cdrom/cdrom.o obj/cdrom/pspcdrom.o
obj/video.a:	obj/video/video.o obj/video/draw.o obj/video/draw_fix.o obj/video/spr.o obj/video/spr_solid.o
#obj/video.a:	obj/video/video.o obj/video/draw_fix.o
obj/cdaudio.a:	obj/cdaudio/cdaudio.o
obj/input.a:	obj/input/input.o
obj/sound.a:	obj/sound/sound.o obj/sound/streams.o obj/sound/2610intf.o obj/sound/ay8910.o \
		obj/sound/fm.o obj/sound/ymdeltat.o obj/sound/timer.o
# obj/zlib.a:	obj/zlib/adler32.o obj/zlib/compress.o obj/zlib/crc32.o obj/zlib/gzio.o\
# 		obj/zlib/uncompr.o obj/zlib/deflate.o obj/zlib/trees.o obj/zlib/zutil.o\
# 		obj/zlib/inflate.o obj/zlib/infblock.o obj/zlib/inftrees.o obj/zlib/infcodes.o\
# 		obj/zlib/infutil.o obj/zlib/inffast.o

obj/libmad.a:	obj/libmad/version.o obj/libmad/fixed.o obj/libmad/bit.o obj/libmad/timer.o \
		obj/libmad/stream.o obj/libmad/frame.o obj/libmad/synth.o obj/libmad/decoder.o \
		obj/libmad/layer12.o obj/libmad/layer3.o obj/libmad/huffman.o

pspclean:
		@rm -rf obj

obj/%.o:	src/%.c
		$(CC) $(CDEFS) $(CFLAGS) -c $< -o$@

obj/%.a:
		$(AR) -r $@ $^

mkdir:
		$(MKDIR) obj
		$(MKDIR) obj/68k
		$(MKDIR) obj/68k/c68k
		$(MKDIR) obj/memory
		$(MKDIR) obj/memory/c68k
		$(MKDIR) obj/cdrom
		$(MKDIR) obj/video
		$(MKDIR) obj/cdaudio
		$(MKDIR) obj/input
		$(MKDIR) obj/z80
		$(MKDIR) obj/z80/mamez80
		$(MKDIR) obj/z80/cz80
		$(MKDIR) obj/sound
		$(MKDIR) obj/zip
#		$(MKDIR) obj/zlib
		$(MKDIR) obj/libmad
		$(MKDIR) obj/psp

# the standard makefile includes ' in the PARAM.SFO target which prevents
# to include bash commands in the version string. Only workaround : make
# my own target.
PARAM.SFO: src/psp/psp.h
	sh make_version

release: EBOOT.PBP kxploit
	$(MKDIR) ~/release
	$(MKDIR) ~/release/1.5
	$(MKDIR) ~/release/1.0
	cp -rffv $(TARGET) $(TARGET)% ~/release/1.5
	cp -f EBOOT.PBP ~/release/1.0
