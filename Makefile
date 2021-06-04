###############################################################################
#
# Makefile for quasi88
#
#		必ず、GNU make が必要です。
#
###############################################################################

#######################################################################
# 基本設定
#######################################################################


# ROM のありかを設定します
#	BASIC の ROMイメージ を検索するディレクトリです。
#	通常は環境変数 $(QUASI88_ROM_DIR) で設定したディレクトリを検索
#	しますが、その環境変数が設定されていない時は、ここを検索します。
#	( ~/ は、QUASI88の起動時にホームディレクトリに展開されます )

ROMDIR	= ./


# DISK のありかを設定します
#	起動時に、引数で指定したイメージファイルを開く際に、
#	そのファイルを検索するディレクトリです。
#	通常は環境変数 $(QUASI88_DISK_DIR) で設定したディレクトリを検索
#	しますが、その環境変数が設定されていない時は、ここを検索します。
#	( なお、ここで見つからなければカレントディレクトリを検索します )
#	( ~/ は、QUASI88の起動時にホームディレクトリに展開されます )

DISKDIR	= ./


# ウェイトの方法を設定します
#	以下をコメントアウトすると、エミュレータのウェイト調整に、SDL の
#	タイマ機能を使います。( サウンドにノイズがのるかもしれません )
#
#	コメントアウトしない場合は、UNIXのシステムコールである、
#	gettimeofday() と select() を使って、ウェイトを調整します。

# UNIX_WAIT = 1



# QUASI88 ではメニューモードにてスクリーンスナップショット(画面キャプチャ)
# の保存が可能ですが、この時に予め指定した任意のコマンドを実行することが
# できます。
# このコマンド実行機能を無効にしたい場合は、以下をコメントアウトして下さい。

SSS_CMD		= -DUSE_SSS_CMD



# モニターモードを有効にする
#	以下のコメントアウトを外すと、デバッグ用のモニターモードが使用
#	できます。通常は特に使うこともないのでコメントアウトのままに
#	しておいても構いません。

# MONITOR = -DUSE_DEBUG



# PC8801のキーボードバグをエミュレートしたい場合は、
# 以下をのコメントアウトを外して下さい。

# KEYBOARD_BUG = -DUSE_KEYBOARD_BUG

# アーキテクチャ選択
SYS_ARCH = XXX


#######################################################################
# サウンド関連の設定
#######################################################################

# サウンドの有無のの指定
#	どちらか一つを指定してください。

#---------------------- サウンドあり
ARCH = with_sound
#---------------------- サウンドなし
#ARCH = generic



#######################################################################
# SDLライブラリの設定
#######################################################################

# sdl-config を実行するコマンドを指定してください。
#	通常のOS の場合、sdl-config   のままで大丈夫なはずです。
#	FreeBSD  の場合、sdl12-config などにリネームされていることがあります

#SDL_CFLAGS= -I/msx/include/SDL -D_GNU_SOURCE=1 -D_THREAD_SAFE
#SDL_LDFLAGS=-L/msx/lib -lSDLmain -lSDL -Wl,-framework,Cocoa
# SDL_LDFLAGS= -lmingw32 -lSDLmain -lSDL -mwindows -luser32 -lgdi32 -lwinmm -lcrtdll
SDL_LDFLAGS= -lc 
LDFLAGS = -nostartfiles -Wl,-Ttext=0x08900000
# -D__PSPSIM__
#SDL_CFLAGG=`sdl-config --cflags`
#SDL_LDFLAGS=`sdl-config --libs`



#######################################################################
# fmgen (FM Sound Generator) の設定
#######################################################################

# cisc氏作の、fmgen (FM Sound Generator) を組み込む場合、以下の行の
# コメントアウトを外して下さい。

# USE_FMGEN = -DUSE_FMGEN


# 注意！
#	FM Sound Generator は C++ により実装されています。
#	C++ のコンパイラの設定を以下で行なって下さい。
# 
# 	設定すべき項目は、CXX、CXXFLAGS、CXXLIBS の定義です。
# 



#######################################################################
# コンパイル関連の設定
#######################################################################

# メイク
MAKE	= gmake


# コンパイラの指定
CC = psp-gcc
# CC	= gcc


# 必要に応じて、コンパイルオプション (最適化など) を指定してください
#
#	コンパイラによっては、char を signed char とみなす指定が必要なf場合が
#	あります。例えば、gcc なら -fsigned-char を指定します。
#
CFLAGS = -O3 -DLSB_FIRST -fomit-frame-pointer -mgp32 -mlong32 -msingle-float -mabi=eabi
ASMFLAGS = -mgp32 -mlong32 -msingle-float -mabi=eabi



# コンパイラによっては、インライン関数を使うことが出来ます。
#	以下から、適切なものを一つだけ指定してください。
#-------------------------------------------------- どんなコンパイラでもOK
USEINLINE	= '-DINLINE=static'
#-------------------------------------------------- GCC の場合
# USEINLINE	= '-DINLINE=static __inline__'
#-------------------------------------------------- Intel C++ コンパイラの場合
# USEINLINE	= '-DINLINE=static inline'
#--------------------------------------------------


# C++ コンパイラの設定
#
#	この設定が必要なのは、
#	『サウンド関連の設定』にて OSの指定を generic 以外にした場合、かつ
#	『fmgen (FM Sound Generator) の設定』で、fmgen を組み込む
#	ように指定した場合のみです。

CXX	 = g++
CXXFLAGS = 
CXXLIBS	 = -lstdc++


# リンカの設定
#	C++ コンパイラを使う場合、環境によっては $(CXX) とする必要が
#	あるかもしれません。

LD	= $(CC)


#######################################################################
# インストールの設定
#######################################################################

# インストール先ディレクトリの設定
#

BINDIR = /usr/local/bin



###############################################################################
#
# これ以降は、変更不要のはずです………
#
###############################################################################

# ファイル snddrv/ignore-opt.c が存在しない場合は、以下をコメントアウトする。
EXIST_IGNORE_OPT = 1


ifdef	UNIX_WAIT
CONFIG  += -DUNIX_WAIT
endif

#######################################################################
# サウンドが有効になっている場合の各種定義
#######################################################################
ifneq ($(ARCH),generic)

#
# サウンド有効時の、追加オブジェクト ( OS依存部 + 共用部 )
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
# サウンド有効時のコンパイルオプション
#

SOUND_OPT		= -DUSE_SOUND -I$(SRCDIR)/$(SNDDRV_DIR)


#
# サウンド有効時のライブラリ指定
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
