###############################################################################
#
# Makefile for quasi88
#
#		ɬ����GNU make ��ɬ�פǤ���
#
###############################################################################

#######################################################################
# ��������
#######################################################################


# ROM �Τ��꤫�����ꤷ�ޤ�
#	BASIC �� ROM���᡼�� �򸡺�����ǥ��쥯�ȥ�Ǥ���
#	�̾�ϴĶ��ѿ� $(QUASI88_ROM_DIR) �����ꤷ���ǥ��쥯�ȥ�򸡺�
#	���ޤ��������δĶ��ѿ������ꤵ��Ƥ��ʤ����ϡ������򸡺����ޤ���
#	( ~/ �ϡ�QUASI88�ε�ư���˥ۡ���ǥ��쥯�ȥ��Ÿ������ޤ� )

ROMDIR	= ./


# DISK �Τ��꤫�����ꤷ�ޤ�
#	��ư���ˡ������ǻ��ꤷ�����᡼���ե�����򳫤��ݤˡ�
#	���Υե�����򸡺�����ǥ��쥯�ȥ�Ǥ���
#	�̾�ϴĶ��ѿ� $(QUASI88_DISK_DIR) �����ꤷ���ǥ��쥯�ȥ�򸡺�
#	���ޤ��������δĶ��ѿ������ꤵ��Ƥ��ʤ����ϡ������򸡺����ޤ���
#	( �ʤ��������Ǹ��Ĥ���ʤ���Х����ȥǥ��쥯�ȥ�򸡺����ޤ� )
#	( ~/ �ϡ�QUASI88�ε�ư���˥ۡ���ǥ��쥯�ȥ��Ÿ������ޤ� )

DISKDIR	= ./


# �������Ȥ���ˡ�����ꤷ�ޤ�
#	�ʲ��򥳥��ȥ����Ȥ���ȡ����ߥ�졼���Υ�������Ĵ���ˡ�SDL ��
#	�����޵�ǽ��Ȥ��ޤ���( ������ɤ˥Υ������Τ뤫�⤷��ޤ��� )
#
#	�����ȥ����Ȥ��ʤ����ϡ�UNIX�Υ����ƥॳ����Ǥ��롢
#	gettimeofday() �� select() ��Ȥäơ��������Ȥ�Ĵ�����ޤ���

# UNIX_WAIT = 1



# QUASI88 �Ǥϥ�˥塼�⡼�ɤˤƥ����꡼�󥹥ʥåץ���å�(���̥���ץ���)
# ����¸����ǽ�Ǥ��������λ���ͽ����ꤷ��Ǥ�դΥ��ޥ�ɤ�¹Ԥ��뤳�Ȥ�
# �Ǥ��ޤ���
# ���Υ��ޥ�ɼ¹Ե�ǽ��̵���ˤ��������ϡ��ʲ��򥳥��ȥ����Ȥ��Ʋ�������

SSS_CMD		= -DUSE_SSS_CMD



# ��˥����⡼�ɤ�ͭ���ˤ���
#	�ʲ��Υ����ȥ����Ȥ򳰤��ȡ��ǥХå��ѤΥ�˥����⡼�ɤ�����
#	�Ǥ��ޤ����̾���ä˻Ȥ����Ȥ�ʤ��Τǥ����ȥ����ȤΤޤޤ�
#	���Ƥ����Ƥ⹽���ޤ���

# MONITOR = -DUSE_DEBUG



# PC8801�Υ����ܡ��ɥХ��򥨥ߥ�졼�Ȥ��������ϡ�
# �ʲ���Υ����ȥ����Ȥ򳰤��Ʋ�������

# KEYBOARD_BUG = -DUSE_KEYBOARD_BUG

# �������ƥ���������
SYS_ARCH = XXX


#######################################################################
# ������ɴ�Ϣ������
#######################################################################

# ������ɤ�̵ͭ�Τλ���
#	�ɤ��餫��Ĥ���ꤷ�Ƥ���������

#---------------------- ������ɤ���
ARCH = with_sound
#---------------------- ������ɤʤ�
#ARCH = generic



#######################################################################
# SDL�饤�֥�������
#######################################################################

# sdl-config ��¹Ԥ��륳�ޥ�ɤ���ꤷ�Ƥ���������
#	�̾��OS �ξ�硢sdl-config   �Τޤޤ�����פʤϤ��Ǥ���
#	FreeBSD  �ξ�硢sdl12-config �ʤɤ˥�͡��व��Ƥ��뤳�Ȥ�����ޤ�

#SDL_CFLAGS= -I/msx/include/SDL -D_GNU_SOURCE=1 -D_THREAD_SAFE
#SDL_LDFLAGS=-L/msx/lib -lSDLmain -lSDL -Wl,-framework,Cocoa
# SDL_LDFLAGS= -lmingw32 -lSDLmain -lSDL -mwindows -luser32 -lgdi32 -lwinmm -lcrtdll
SDL_LDFLAGS= -lc 
LDFLAGS = -nostartfiles -Wl,-Ttext=0x08900000
# -D__PSPSIM__
#SDL_CFLAGG=`sdl-config --cflags`
#SDL_LDFLAGS=`sdl-config --libs`



#######################################################################
# fmgen (FM Sound Generator) ������
#######################################################################

# cisc���Ρ�fmgen (FM Sound Generator) ���Ȥ߹����硢�ʲ��ιԤ�
# �����ȥ����Ȥ򳰤��Ʋ�������

# USE_FMGEN = -DUSE_FMGEN


# ��ա�
#	FM Sound Generator �� C++ �ˤ���������Ƥ��ޤ���
#	C++ �Υ���ѥ���������ʲ��ǹԤʤäƲ�������
# 
# 	���ꤹ�٤����ܤϡ�CXX��CXXFLAGS��CXXLIBS ������Ǥ���
# 



#######################################################################
# ����ѥ����Ϣ������
#######################################################################

# �ᥤ��
MAKE	= gmake


# ����ѥ���λ���
CC = psp-gcc
# CC	= gcc


# ɬ�פ˱����ơ�����ѥ��륪�ץ���� (��Ŭ���ʤ�) ����ꤷ�Ƥ�������
#
#	����ѥ���ˤ�äƤϡ�char �� signed char �Ȥߤʤ����꤬ɬ�פ�f��礬
#	����ޤ����㤨�С�gcc �ʤ� -fsigned-char ����ꤷ�ޤ���
#
CFLAGS = -O3 -DLSB_FIRST -fomit-frame-pointer -mgp32 -mlong32 -msingle-float -mabi=eabi
ASMFLAGS = -mgp32 -mlong32 -msingle-float -mabi=eabi



# ����ѥ���ˤ�äƤϡ�����饤��ؿ���Ȥ����Ȥ�����ޤ���
#	�ʲ����顢Ŭ�ڤʤ�Τ��Ĥ������ꤷ�Ƥ���������
#-------------------------------------------------- �ɤ�ʥ���ѥ���Ǥ�OK
USEINLINE	= '-DINLINE=static'
#-------------------------------------------------- GCC �ξ��
# USEINLINE	= '-DINLINE=static __inline__'
#-------------------------------------------------- Intel C++ ����ѥ���ξ��
# USEINLINE	= '-DINLINE=static inline'
#--------------------------------------------------


# C++ ����ѥ��������
#
#	�������꤬ɬ�פʤΤϡ�
#	�إ�����ɴ�Ϣ������٤ˤ� OS�λ���� generic �ʳ��ˤ�����硢����
#	��fmgen (FM Sound Generator) ������٤ǡ�fmgen ���Ȥ߹���
#	�褦�˻��ꤷ�����ΤߤǤ���

CXX	 = g++
CXXFLAGS = 
CXXLIBS	 = -lstdc++


# ��󥫤�����
#	C++ ����ѥ����Ȥ���硢�Ķ��ˤ�äƤ� $(CXX) �Ȥ���ɬ�פ�
#	���뤫�⤷��ޤ���

LD	= $(CC)


#######################################################################
# ���󥹥ȡ��������
#######################################################################

# ���󥹥ȡ�����ǥ��쥯�ȥ������
#

BINDIR = /usr/local/bin



###############################################################################
#
# ����ʹߤϡ��ѹ����פΤϤ��Ǥ��ġġ�
#
###############################################################################

# �ե����� snddrv/ignore-opt.c ��¸�ߤ��ʤ����ϡ��ʲ��򥳥��ȥ����Ȥ��롣
EXIST_IGNORE_OPT = 1


ifdef	UNIX_WAIT
CONFIG  += -DUNIX_WAIT
endif

#######################################################################
# ������ɤ�ͭ���ˤʤäƤ�����γƼ����
#######################################################################
ifneq ($(ARCH),generic)

#
# �������ͭ�����Ρ��ɲå��֥������� ( OS��¸�� + ������ )
#

SNDDRV_DIR	= snddrv
QUASI_DIR	= $(SNDDRV_DIR)

SRC_DIR		= $(SNDDRV_DIR)/
SOUND_DIR	= $(SNDDRV_DIR)/
DSP_DIR		= $(SNDDRV_DIR)/


SNDDRV_OBJS	= $(SNDDRV_DIR)/q88psp_snd.o	\
		  $(SNDDRV_DIR)/beepintf.o	\
		  $(SNDDRV_DIR)/beep.o		\
		  $(SNDDRV_DIR)/sndintrf.o	\
		  $(SNDDRV_DIR)/2203intf.o	\
		  $(SNDDRV_DIR)/2608intf.o	\
		  $(SNDDRV_DIR)/ay8910.o	\
		  $(SNDDRV_DIR)/fm.o		\
		  $(SNDDRV_DIR)/ymdeltat.o	\
		  $(SNDDRV_DIR)/filter.o	\
		  $(SNDDRV_DIR)/mixer.o		\
		  $(SNDDRV_DIR)/streams.o	\
		  $(SNDDRV_DIR)/sdl_xxx.o



#
# �������ͭ�����Υ���ѥ��륪�ץ����
#

SOUND_OPT		= -DUSE_SOUND -I$(SRCDIR)/$(SNDDRV_DIR)


#
# �������ͭ�����Υ饤�֥�����
#

SOUND_LIB		= -lm


ifdef	USE_FMGEN

FMGEN_DIR	= fmgen
FMGEN_OBJ	= $(FMGEN_DIR)/2203fmgen.o	\
		  $(FMGEN_DIR)/2608fmgen.o	\
		  $(FMGEN_DIR)/fmgen.o		\
		  $(FMGEN_DIR)/fmtimer.o	\
		  $(FMGEN_DIR)/opna.o		\
		  $(FMGEN_DIR)/psg.o

SOUND_OPT	+= -I$(SRCDIR)/$(FMGEN_DIR)

SNDDRV_OBJS	+= $(FMGEN_OBJ)

MY_LIBS		+= $(CXXLIBS)

endif

endif


ifdef EXIST_IGNORE_OPT
IGNORE_OPT_C	= snddrv/ignore-opt.c
IGNORE_OPT      = -DEXIST_IGNORE_OPT_C
endif


#######################################################################
#
#######################################################################


# NAME    = QUASI88(SDL)
# VERSION = 0.0.0s0
PROGRAM = out

INC_MINGW = -I/mingw/include
DIR_PSP = pspsys

PSPSIM = $(DIR_PSP)/pspsim.o \
		 $(DIR_PSP)/pspsys.o \
		 $(DIR_PSP)/startsim.o

FILEOP = $(SYS_ARCH)/file-op.o

# PSPDEP =
PSPDEP = startup.o syscall.o 

OBJECT =  $(PSPDEP) \
    quasi88.o getconf.o emu.o graph_xxx.o exmem.o memory.o indicator.o wait_xxx.o \
	pc88main.o crtcdmac.o sound.o pio.o screen.o intr.o keyboard_xxx.o \
	pc88sub.o fdc.o image.o $(FILEOP) monitor.o monitor-event.o pause_xxx.o \
	menu.o menu-event_xxx.o menu-screen.o q8tk-core.o q8tk-glib.o suspend.o\
	z80.o z80-debug.o joystick_xxx.o snapshot.o pg.o \
	$(SNDDRV_OBJS) 


# $(PSPSIM)


CFLAGS += -Isrc $(USEINLINE) \
	  -DROM_DIR='"$(ROMDIR)"' -DDISK_DIR='"$(DISKDIR)"' \
	  $(READLINE) $(IGNORE_C_D) $(GETTIMEOFDAY) \
	  -DBIT32='$(TYPE_SPECIFIER_OF_32BIT)' $(IGNORE_OPT) \
	  $(MONITOR) $(SSS_CMD) $(USELOCALE) $(KEYBOARD_BUG) $(USE_FMGEN) \
	  $(SDL_CFLAGS) -DUNIX88 \
	  -D__ARCH_$(ARCH) -DPI=M_PI $(SOUND_OPT) $(CONFIG)

CXXFLAGS += -Isrc $(USEINLINE) \
	  -DROM_DIR='"$(ROMDIR)"' -DDISK_DIR='"$(DISKDIR)"' \
	  $(READLINE) $(IGNORE_C_D) $(GETTIMEOFDAY) \
	  -DBIT32='$(TYPE_SPECIFIER_OF_32BIT)' $(IGNORE_OPT) \
	  $(MONITOR) $(SSS_CMD) $(USELOCALE) $(KEYBOARD_BUG) $(USE_FMGEN) \
	  $(SDL_CFLAGS) -DUNIX88 \
	  -D__ARCH_$(ARCH) -DPI=M_PI $(SOUND_OPT) $(CONFIG)

LIBS   = $(READLINELIB) $(SOUND_LIB) $(SDL_LDFLAGS) $(MY_LIBS)

###

OBJDIR		= obj
SRCDIR		= src

OBJDIRS		= $(OBJDIR) $(OBJDIR)/$(SYS_ARCH) \
		  $(addprefix $(OBJDIR)/, $(SNDDRV_DIR) $(FMGEN_DIR) $(DIR_PSP))

OBJECTS		= $(addprefix $(OBJDIR)/, $(OBJECT) )

###

all:		$(OBJDIRS) $(PROGRAM)

$(OBJDIRS):
		-mkdir $@

$(PROGRAM):	$(OBJECTS)
		$(LD)  $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(PROGRAM) > map.txt
		outpatch
		psp-strip outp
		elf2pbp outp "Q88PSP"



$(OBJDIR)/$(SYS_ARCH)/%.o: $(SRCDIR)/$(SYS_ARCH)/%.c
		$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/$(SNDDRV_DIR)/%.o: $(SRCDIR)/$(SNDDRV_DIR)/%.c
		$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/$(DIR_PSP)/%.o: $(SRCDIR)/$(DIR_PSP)/%.c
		$(CC) $(INC_MINGW) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
		$(CC) $(CFLAGS) -o $@ -c $<


$(OBJDIR)/%.o: $(SRCDIR)/%.S
		$(CC) $(ASMFLAGS) -c $< -o $@


$(OBJDIR)/$(FMGEN_DIR)/%.o: $(SRCDIR)/$(FMGEN_DIR)/%.cpp
		$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
		rm -rf $(OBJDIR) $(PROGRAM) $(PROGRAM).core


install:
		@echo installing binaries under $(BINDIR)...
		@cp $(PROGRAM) $(BINDIR)

#
#
#
