#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

extern	int	menu_lang;			/* メニューの言語	*/
static int MenuMode;
static int KeySet;
static int DiskIn[2];
extern char *Drive1;
extern char *Drive2;
#define MAXPATH 0x108
extern char	path_main[MAXPATH];
//extern char	SetDisk_path[MAXPATH];

#define KeyConfig
#define SoftwareKeybord
/*----------------------------------------------------------------------*/
/* キーコード変換表							*/
/*----------------------------------------------------------------------*/
//keyboard.cから移動

#define Bt7	0x80
#define Bt6	0x40
#define Bt5	0x20
#define Bt4	0x10
#define Bt3	0x08
#define Bt2	0x04
#define Bt1	0x02
#define Bt0	0x01

#define P0	0
#define P1	1
#define P2	2
#define P3	3
#define P4	4
#define P5	5
#define P6	6
#define P7	7
#define P8	8
#define P9	9
#define Pa	10
#define Pb	11
#define Pc	12
#define Pd	13
#define Pe	14

static int Menu_y[] = {
	2,
	4,
	6,
	8,
	10,
	12,
	14,
	16,
	18,
	20,
	22,
	24,
	26,
	28,
	30,
	32,
	34,
	36,
};

static int Menu_x[] = {
	5,
	7,
	17,
	27,
	37,
	47,
};

static int Clock_main[] = {
	222,
	266,
	333,
};

static int Clock_sub[] = {
	111,
	133,
	166,
};

enum{
	Control_Up,
	Control_Down,
	Control_Left,
	Control_Right,
	Control_Circle,
	Control_Cross,
	Control_Triangle,
	Control_Square,
	Control_L,
	Control_R,
	Control_Start,
	Control_Select,
	Control_End,
};

static long Control_val[] = {
	0x000010,//#define CTRL_UP			
	0x000040,//#define CTRL_DOWN		
	0x000080,//#define CTRL_LEFT		
	0x000020,//#define CTRL_RIGHT		
	0x002000,//#define CTRL_CIRCLE		
	0x004000,//#define CTRL_CROSS		
	0x001000,//#define CTRL_TRIANGLE	
	0x008000,//#define CTRL_SQUARE		
	0x000100,//#define CTRL_LTRIGGER	
	0x000200,//#define CTRL_RTRIGGER	
	0x000008,//#define CTRL_START		
	0x000001,//#define CTRL_SELECT		
	0x020000,//#define CTRL_HOLD		
};

enum{
	Menu_pspclock,	// 0
	Menu_keyconfig,	// 1
	Menu_clock,		// 2
	Menu_mode,		// 3
	Menu_subcpu,	// 4
	Menu_palette,	// 5
	Menu_size		,// 6
	Menu_half_interp,// 7
	Menu_drive1,	// 8
	Menu_drive2,	// 9
	Menu_power,		// 10
	Menu_continue,	// 11
	Menu_state,		// 12
	Menu_exit,		// 13
};

static struct {
	int Port;
	int Bit;
} PSPControl[] = {
	//Default Config
	{ P1 ,Bt0},// Up -> 8(TenKey)
	{ P0 ,Bt2},// Down -> 2(TenKey)
	{ P0 ,Bt4},// Left -> 4(TenKey)
	{ P0 ,Bt6},// Right -> 6(TenKey)
	{ P9 ,Bt6},// Circle -> Space
	{ P1 ,Bt7},// Cross -> Return
	{ Pc ,Bt7},// Triangle -> Delete
	{ P4 ,Bt3},// Square -> 's'
	{ 0  ,0  },// LTrigger -> Menu(No Key Setting)
	{ P8 ,Bt6},// RTrigger -> SHIFT
	{ P9 ,Bt1},// Start -> F1
	{ P8 ,Bt3},// Select -> DELETE
};

#define KeyCol 21
static char *KeyBorad[] = {
//Standard
	// 0    1    2    3    4    5    6    7    8    9   10   11    12    13   14   15  16   17   18   19
	"ST","CO","F1","F2","F3","F4","F5","F6","F7","F8","F9", "FA",  "",   "",  "","PU","PD","HC","HP"," -"," /",
	"ES"," 1"," 2"," 3"," 4"," 5"," 6"," 7"," 8"," 9"," 0", " -"," ^"," \\","BS","IN","DE"," 7"," 8"," 9"," *",
	"TB"," q"," w"," e"," r"," t"," y"," u"," i"," o"," p", " @"," [", "RT",  "",  "",  ""," 4"," 5"," 6"," +",
	"CL","CA"," a"," s"," d"," f"," g"," h"," j"," k"," l", " ;"," :", " ]",  "","UP",  ""," 1"," 2"," 3"," =",
	"SH"," z"," x"," c"," v"," b"," n"," m"," ,"," ."," /"," \\",  "",   "","LE","DO","RI"," 0"," ,"," .","RT",
	"KA","GR",  "",  "","SP","SP","SP",  "",  "","PC",  "",   "",  "",   "",  "",  "",  "",  "",  "",  "","  ",
	"SP",
//CAPS
	// 0    1    2    3    4    5    6    7    8    9   10   11    12    13   14   15  16   17   18   19
	"ST","CO","F1","F2","F3","F4","F5","F6","F7","F8","F9", "FA",  "",   "",  "","PU","PD","HC","HP"," -"," /",
	"ES"," 1"," 2"," 3"," 4"," 5"," 6"," 7"," 8"," 9"," 0", " -"," ^"," \\","BS","IN","DE"," 7"," 8"," 9"," *",
	"TB"," Q"," W"," E"," R"," T"," Y"," U"," I"," O"," P", " @"," [", "RT",  "",  "",  ""," 4"," 5"," 6"," +",
	"CL","CA"," A"," S"," D"," F"," G"," H"," J"," K"," L", " ;"," :", " ]",  "","UP",  ""," 1"," 2"," 3"," =",
	"SH"," Z"," X"," C"," V"," B"," N"," M"," ,"," ."," /"," \\",  "",   "","LE","DO","RI"," 0"," ,"," .","RT",
	"KA","GR",  "",  "","SP","SP","SP",  "",  "","PC",  "",   "",  "",   "",  "",  "",  "",  "",  "",  "","  ",
	"SP",
//KANA
	// 0    1    2    3    4    5    6    7    8    9   10   11    12    13   14   15  16   17   18   19
	"ST","CO","F1","F2","F3","F4","F5","F6","F7","F8","F9", "FA",  "",   "",  "","PU","PD","HC","HP"," -"," /",
	"ES"," ﾇ"," ﾌ"," ｱ"," ｳ"," ｴ"," ｵ"," ﾔ"," ﾕ"," ﾖ"," ﾜ", " ﾎ"," ﾍ", " ｰ","BS","IN","DE"," 7"," 8"," 9"," *",
	"TB"," ﾀ"," ﾃ"," ｲ"," ｽ"," ｶ"," ﾝ"," ﾅ"," ﾆ"," ﾗ"," ｾ", " ﾞ"," ﾟ", "RT",  "",  "",  ""," 4"," 5"," 6"," +",
	"CL","CA"," ﾁ"," ﾄ"," ｼ"," ﾊ"," ｷ"," ｸ"," ﾏ"," ﾉ"," ﾘ", " ﾚ"," ｹ", " ﾑ",  "","UP",  ""," 1"," 2"," 3"," =",
	"SH"," ﾂ"," ｻ"," ｿ"," ﾋ"," ｺ"," ﾐ"," ﾓ"," ﾈ"," ﾙ"," ﾒ", " ﾛ",  "",   "","LE","DO","RI"," 0"," ,"," .","RT",
	"KA","GR",  "",  "","SP","SP","SP",  "",  "","PC",  "",   "",  "",   "",  "",  "",  "",  "",  "",  "","  ",
	"SP",
//SHIFT
	// 0    1    2    3    4    5    6    7    8    9   10   11    12    13   14   15  16   17   18   19
	"ST","CO","F1","F2","F3","F4","F5","F6","F7","F8","F9", "FA",  "",   "",  "","PU","PD","HC","HP"," -"," /",
	"ES"," !"," \""," #"," $"," %"," &"," '"," ("," )"," 0", " ="," ~"," |","BS","IN","DE"," 7"," 8"," 9"," *",
	"TB"," Q"," W"," E"," R"," T"," Y"," U"," I"," O"," P", " ~"," {", "RT",  "",  "",  ""," 4"," 5"," 6"," +",
	"CL","CA"," A"," S"," D"," F"," G"," H"," J"," K"," L", " +"," *", " }",  "","UP",  ""," 1"," 2"," 3"," =",
	"SH"," Z"," X"," C"," V"," B"," N"," M"," <"," >"," ?", " |",  "",   "","LE","DO","RI"," 0"," ,"," .","RT",
	"KA","GR",  "",  "","SP","SP","SP",  "",  "","PC",  "",   "",  "",   "",  "",  "",  "",  "",  "",  "","  ",
	"SP",
//CAPS + SHIFT
	// 0    1    2    3    4    5    6    7    8    9   10   11    12    13   14   15  16   17   18   19
	"ST","CO","F1","F2","F3","F4","F5","F6","F7","F8","F9", "FA",  "",   "",  "","PU","PD","HC","HP"," -"," /",
	"ES"," !"," \""," #"," $"," %"," &"," '"," ("," )"," 0", " ="," ~"," |","BS","IN","DE"," 7"," 8"," 9"," *",
	"TB"," q"," w"," e"," r"," t"," y"," u"," i"," o"," p", " ~"," {", "RT",  "",  "",  ""," 4"," 5"," 6"," +",
	"CL","CA"," a"," s"," d"," f"," g"," h"," j"," k"," l", " +"," *", " }",  "","UP",  ""," 1"," 2"," 3"," =",
	"SH"," z"," x"," c"," v"," b"," n"," m"," <"," >"," ?", " |",  "",   "","LE","DO","RI"," 0"," ,"," .","RT",
	"KA","GR",  "",  "","SP","SP","SP",  "",  "","PC",  "",   "",  "",   "",  "",  "",  "",  "",  "",  "","  ",
	"SP",
//KANA + SHIFT
	// 0    1    2    3    4    5    6    7    8    9   10   11    12    13   14   15  16   17   18   19
	"ST","CO","F1","F2","F3","F4","F5","F6","F7","F8","F9", "FA",  "",   "",  "","PU","PD","HC","HP"," -"," /",
	"ES"," ﾇ"," ﾌ"," ｧ"," ｩ"," ｪ"," ｫ"," ｬ"," ｭ"," ｮ"," ﾜ", " ﾎ"," ﾍ", " ｰ","BS","IN","DE"," 7"," 8"," 9"," *",
	"TB"," ﾀ"," ﾃ"," ｨ"," ｽ"," ｶ"," ﾝ"," ﾅ"," ﾆ"," ﾗ"," ｾ", " ﾞ"," ｢", "RT",  "",  "",  ""," 4"," 5"," 6"," +",
	"CL","CA"," ﾁ"," ﾄ"," ｼ"," ﾊ"," ｷ"," ｸ"," ﾏ"," ﾉ"," ﾘ", " ﾚ"," ｹ", " ｣",  "","UP",  ""," 1"," 2"," 3"," =",
	"SH"," ｯ"," ｻ"," ｿ"," ﾋ"," ｺ"," ﾐ"," ﾓ"," ﾈ"," ﾙ"," ･", " ｰ",  "",   "","LE","DO","RI"," 0"," ,"," .","RT",
	"KA","GR",  "",  "","SP","SP","SP",  "",  "","PC",  "",   "",  "",   "",  "",  "",  "",  "",  "",  "","  ",
	"SP",
};

static char *Number[] = {" 0"," 1"," 2"," 3"," 4"," 5"," 6"," 7"," 8"," 9","10","11","12","13","14","15" };

static struct {
	int Port;
	int Bit;
} KeyInput[] = {
//	  STOP     COPY       f1       f2      f3       f4       f5        f6       f7      f8       f9       f10      N/A      N/A      N/A     R UP     R DN      HOME     HELP     -[10]   /[10]
	{P9,Bt0},{Pa,Bt4},{P9,Bt1},{P9,Bt2},{P9,Bt3},{P9,Bt4},{P9,Bt5},{P9,Bt1},{P9,Bt2},{P9,Bt3},{P9,Bt4},{P9,Bt5},{ 0,  0},{ 0,  0},{ 0,  0},{Pb,Bt0},{Pb,Bt1},{P8,Bt0},{Pa,Bt3},{Pa,Bt5},{Pa,Bt6},
//	  "ES",    " 1",    " 2",    " 3",    " 4",    " 5",    " 6",    " 7",    " 8",    " 9",    " 0",    " -",    " ^",    " \\",    "BS",   "IN",    "DE",    " 7",    " 8",    " 9",    " *"
//	{P9,Bt7},{P6,Bt1},{P6,Bt2},{P6,Bt3},{P6,Bt4},{P6,Bt5},{P6,Bt6},{P6,Bt7},{P7,Bt0},{P7,Bt1},{P6,Bt0},{P5,Bt7},{P5,Bt6},{P5,Bt4},{Pc,Bt5},{Pc,Bt6},{Pc,Bt7},{P0,Bt7},{P1,Bt0},{P1,Bt1},{P1,Bt2},
	{P9,Bt7},{P6,Bt1},{P6,Bt2},{P6,Bt3},{P6,Bt4},{P6,Bt5},{P6,Bt6},{P6,Bt7},{P7,Bt0},{P7,Bt1},{P6,Bt0},{P5,Bt7},{P5,Bt6},{P5,Bt4},{P8,Bt3},{P8,Bt3},{P8,Bt3},{P0,Bt7},{P1,Bt0},{P1,Bt1},{P1,Bt2},//多分これでDelete、Backspaceが効くと思う
//	  "TB",    " q",    " w",    " e",    " r",    " t",    " y",    " u",    " i",    " o",    " p",    " @",    " [",     "RT",     "",      "",      "",    " 4",    " 5",    " 6",    " +"
	{Pa,Bt0},{P4,Bt1},{P4,Bt7},{P2,Bt5},{P4,Bt2},{P4,Bt4},{P5,Bt1},{P4,Bt5},{P3,Bt1},{P3,Bt7},{P4,Bt0},{P2,Bt0},{P5,Bt3},{P1,Bt7},{ 0,  0},{ 0,  0},{ 0,  0},{P0,Bt4},{P0,Bt5},{P0,Bt6},{P1,Bt3},
//	  "CL",    "CA",    " a",    " s",    " d",    " f",    " g",    " h",    " j",    " k",    " l",    " ;",    " :",     " ]",     "",    "UP",      "",    " 1",    " 2",    " 3",    " ="
	{P8,Bt7},{Pa,Bt7},{P2,Bt1},{P4,Bt3},{P2,Bt4},{P2,Bt6},{P2,Bt7},{P3,Bt0},{P3,Bt2},{P3,Bt3},{P3,Bt4},{P7,Bt3},{P7,Bt2},{P5,Bt5},{ 0,  0},{P8,Bt1},{ 0,  0},{P0,Bt1},{P0,Bt2},{P0,Bt3},{P1,Bt4},
//	  "SH",    " z",    " x",    " c",    " v",    " b",    " n",    " m",    " ,",    " .",    " /",    " \\",     "",     "",     "LT",    "DN"    ,"RT",    " 0",    " ,",    " .",    "RT"
	{P8,Bt6},{P5,Bt2},{P5,Bt0},{P2,Bt3},{P4,Bt6},{P2,Bt2},{P3,Bt6},{P3,Bt5},{P7,Bt4},{P7,Bt5},{P7,Bt6},{P5,Bt4},{ 0,  0},{ 0,  0},{Pa,Bt2},{Pa,Bt1},{P8,Bt2},{P0,Bt0},{P1,Bt5},{P1,Bt6},{P1,Bt7},
//	  "KA"     "GR"      N/A      N/A     "SP"     "SP"     "SP"      N/A      N/A     "PC"      N/A      N/A      N/A      N/A      N/A      N/A      N/A      N/A      N/A      N/A      N/A   
	{P8,Bt5},{P8,Bt4},{ 0,  0},{ 0,  0},{P9,Bt6},{P9,Bt6},{P9,Bt6},{ 0,  0},{ 0,  0},{Pd,Bt2},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},{ 0,  0},
//	  "SP"
	{P9,Bt6},
};

typedef struct{
	int PSPClock;
	int Clock;
	int Mode;
	int Subcpu;
	int Palette;
	int Size;
	int Half;
	char Image[1];
	int state;
} _init;

typedef struct {
	char CfgVer[2];
	int PSPClock;
	int Clock;
	int Mode;
	int KeySetting[12];
	char File[2][0x108];
	int DiskIn[2];
	char Image[1];
	int Subcpu;
	int Palette;
	int Half;
	int Size;
} _cfg;//変数の並べ替え、挿入、削除不可。コンフィグロード時にハングアップする可能性あり。

typedef struct 
{
	unsigned short unk[2];
	unsigned short year;
	unsigned short mon;
	unsigned short mday;
	unsigned short hour;
	unsigned short min;
	unsigned short sec;
} psp_de_tm;

typedef struct {
	unsigned long unk1;
	unsigned long type;
	unsigned long size;
	psp_de_tm ctime;
	psp_de_tm atime;
	psp_de_tm mtime;
	unsigned long unk[7];
	char name[0x108];
} psp_dirent;

extern int KeySetting[];
extern int CAPS;
extern int KANA;
extern int SHIFT;
extern int CAPS_KEY;
extern int KANA_KEY;
extern int BS_KEY;
extern int DEL_KEY;
extern int INS_KEY;
extern int F6_KEY;
extern int F7_KEY;
extern int F8_KEY;
extern int F9_KEY;
extern int F10_KEY;
extern int SHIFT_KEY;
extern int SoftKB;
extern int SoftKBSet;
	/* メニューモード */

void	menu_init( void );
void	menu_main( void );
void	menu_draw(int Power);
int		menu_psp(int Power);
void	menu_init_psp( void );
void	cfg_write(void);
void	KeyConfigMenu(void);
void 	KeyConfigDraw(int Mode);
void 	KeyConfigKBDraw(int Mode);
void	KeyConfigKBErace(void);
int		GetFileName(int Drive);
int		ChackName(void);
int		KeyboradMap(void);
#endif	/* MENU_H_INCLUDED */
