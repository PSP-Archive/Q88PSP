/************************************************************************/
/*									*/
/* FDC の制御 と ディスクイメージの読み書き				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "drive.h"
#include "image.h"
#include "fdc.h"

#include "emu.h"		/* emu_mode			*/

#include "file-op.h"
#include "suspend.h"



static int fdc_break_flag = FALSE;

int	fdc_debug_mode = FALSE;	/* FDC デバッグモードのフラグ		*/
int	disk_exchange = FALSE;	/* ディスク疑似入れ替えフラグ		*/
int	disk_ex_drv = 0;	/* ディスク疑似入れ替えドライブ		*/
				/*    drive 1 ... bit 0			*/
				/*    drive 2 ... bit 1			*/
/* 上記処理は、peach氏の提供による */

int	FDC_flag = 0;			/* FDC 割り込み信号		*/
int	fdc_wait = 0;			/* FDC の ウエイト 0無 1有	*/


#define	MAX_DRIVE	(4)		/* FDC で制御可能な最大ドライブ数 */
					/* 実際には、NR_DRIVE(==2)だけに対応 */


/*
  ディスクをアクセス中に抜き出した際の挙動

  ID を検索する前 → INDEX パルスを検出しないので、いつまでたっても終らない
  ID 読み込み中   → ID CRC エラー発生
  ID と DATA の間 → DDAM が一定時間内に見つからないエラー
  DATA 読み込み中 → DATA CRC エラー
  書き込み動作    → なにもなく終る。(でも、読む時にCRCエラーのはず)


  エミュレート時にイメージが破壊されていたら
  トラックが無い → オフセットが 0 になっている／EOFに達した
                                 …… アンフォーマットトラックとして扱う
  セクタが無い   → EOTに達した  …… ID が無い時は、ID CRC エラー
                                      DATA が無い時は、DATA CRC エラー
  書き込みエラー → EOTに達した  …… なにも処理せずに終る


  処理
  ディスクセット     → PCNx2 のトラックにシーク
  ディスクイジェクト → ディスク無し設定
  READ ID            → E-Phase に、fseek/fread
  READ DATA          → C-Phase 終了後に fseek/fread し、バッファに読み込む
                        E-Phase で、内容を吐き出す
  WRITE ID           → C-Phase で fseek、E-Phase で、fwrite
  WRITE DATA         → E-Phase で、内容をバッファにため、終了時に fwrite
  READ DAIGNOSTIC    → C-Phase で、バッファに内容を作成、E-Phase で吐き出す
  SEEK/RECALIBRATE   → PCN 更新。ディスクがあれば PCNx2 のトラックにシーク
   C-Phase で同じトラックに対する処理は、セクタの続きをアクセス
             違うトラックに対する処理は、先頭セクタをアクセス



  メッセージ
  WRITE ID 時に、フォーマットサイズが6250バイト以上の時は、注意メッセージを
                 表示する。(書き込みは正常に行なわれる)
		 書き込みサイズが、0x1600バイト(2D)以上の時、(N=6,7での
		 フォーマットを含む)は、警告を表示し、アンフォーマットを作成
  WRITE ID 時に、CHRN と セクタ長 N がことなる時は、注意メッセージを表示
  WRITE DATA 時に、CHRN と セクタ長 N がことなる時は、警告メッセージを表示し、
                   書き込む。(多分、そのディスクはもう使えない)

  複数イメージ
  ディスクを設定する時、すべてのイメージを検索し、名前、属性、サイズを
  ワークに記録しておく。(実際には全てではなく、MAX_NR_IMAGE個まで)
  この時、壊れたイメージ(シークエラー、ヘッダのリードエラー、データサイズが、
  ヘッダ内容と異なる)がある場合は ワーク detect_broken_image を真に設定して
  おき、そのイメージ以降のイメージはアクセスできないようにする。
  なお、エラーが出る以前のイメージにはアクセス可能。

  すでにオープンしているイメージファイルに対する情報変更および、ブランク
  イメージの追加は、detect_broken_image が真の場合は警告をだす。

  情報変更、イメージ追加の際にエラーが出た時は、最悪イメージ破損の可能性が
  あるので、警告をだす。

  アクセス時のチェック。fseek & fwrite の前に、イメージの終端の位置をチェック
  して、間違って次のイメージに書き込まないようにする。これは、壊れたイメージ
  ヘッダに対する策である。fseek & fread の前にもチェックしようかと思ったが、
  どうせ最終トラックに対してしか有効でない方法なので、やめにした。

*/


/*
 * ディスクイメージに関するワーク (ドライブ単位)
 */
PC88_DRIVE_T	drive[ NR_DRIVE ];



/*
 * ディスクイメージからセクタを読んだ時際の、情報を格納
 */
static	struct{
  Uchar   c;			/* セクタ ID の C	*/
  Uchar   h;			/* セクタ ID の H	*/
  Uchar   r;			/* セクタ ID の R	*/
  Uchar   n;			/* セクタ ID の N	*/
  Uchar   density;		/* セクタの記録密度	*/
  Uchar   deleted;		/* DELETED DATA フラグ	*/
  Uchar   status;		/* PC98 BIOSのステータス*/
  Uchar   padding;
  int     sec_nr;		/* トラック内のセクタ数	*/
  int	  size;			/* DATA サイズ		*/
} sec_buf;


/*
 * READ / WRITE 時のデータはここにセット
 *	WRITE ID では、4バイトデータ×セクタ数をここにセットする。
 *	READ DIAGNOSTIC では、ベタデータのイメージがここに作成される。
 */

#define DATA_BUF_SIZE 0x4000	/* 最大 2D/2DD=6250byte、2HD=10416byte 位? */

static	Uchar	data_buf[ DATA_BUF_SIZE ];


/*
 * FDC の各種情報ワーク
 *	ホストから受け取ったコマンド、処理後のステータスは、ここに。
 */
static	struct{

  int	command;		/* コマンド (enum値)	*/
  int	phase;			/* PHASE (C/E/R)	*/
  int	step;			/* PAHSE内の処理手順	*/
  int	counter;		/* 各種カウンタ		*/
  int	data_ptr;		/* データ転送のポインタ	*/

  int	limit;			/* データ転送タイムアウトダウンカウンタ */
  int	wait;			/* 次処理実行開始までのウエイト		*/
  int	seek_wait[MAX_DRIVE];	/* ドライブ別 シークにかかる時間 (4台分)*/

  byte	status;			/* STATUS		*/
  byte	read;			/* DATA  for  FDC->MAIN */
  byte	write;			/* DATA  for  FDC<-MAIN */
  byte	TC;			/* TC (1 or 0 )		*/

  Uchar	cmd;			/* コマンド		*/
  Uchar	sk;			/* SK ビット		*/
  Uchar	mf;			/* MF ビット		*/
  Uchar	mt;			/* MT ビット		*/
  Uchar	us;			/* US 番号		*/
  Uchar	hd;			/* HD 側		*/
  Uchar	c;			/* ID - C		*/
  Uchar	h;			/* ID - H		*/
  Uchar	r;			/* ID - R		*/
  Uchar	n;			/* ID - N		*/
  Uchar	eot;			/* EOT 番号		*/
  Uchar	gpl;			/* GPL 長さ		*/
  Uchar	dtl;			/* DTL 長さ		*/
  Uchar	d;			/* D   データ		*/
  Uchar	sc;			/* SC  セクタ数		*/
  Uchar	stp;			/* STP 間隔		*/
  Uchar	srt_ms;			/* SRT(ms換算)		*/
  Uchar	hut;			/* HUT			*/
  Uchar	hlt_nd;			/* HLT & ND		*/
  Uchar	ncn[MAX_DRIVE];		/* NCN 位置 (4台分)	*/
  Uchar	pcn[MAX_DRIVE];		/* PCN 位置 (4台分)	*/
  Uchar	st0;			/* ST0			*/
  Uchar	st1;			/* ST1			*/
  Uchar	st2;			/* ST2			*/
  Uchar	st3;			/* ST3			*/
  Uchar	st0_seek[MAX_DRIVE];	/* SEEK時のST0 (4台分)	*/
  Uchar	sense_us;		/* SENSE_I_S 処理中 US	*/
  Uchar	skip_ddam_job;		/* スキップ指定時の処理	*/

} fdc;


/* 各種マクロ */


#define	disk_not_exist(drv)	(drive[drv].fp==NULL  || drive[drv].empty)
#define	disk_unformat(drv)	(drive[drv].sec_nr<=0 || drive[drv].empty)
#define disk_unformatable(drv)	(drive[drv].sec_nr<0  || drive[drv].empty)

#define	sector_density_mismatch()					\
		(((sec_buf.density==DISK_DENSITY_SINGLE)&&( fdc.mf))||	\
		 ((sec_buf.density==DISK_DENSITY_DOUBLE)&&(!fdc.mf)) )

#define	idr_match()							\
		( fdc.c==sec_buf.c && fdc.h==sec_buf.h &&		\
		  fdc.r==sec_buf.r && fdc.n==sec_buf.n )

#define	printf_system_error( code )					      \
    do{									      \
      if     ( code==1 ) pgPrint_drawbg(3,32,0xFFFF,0,"FDC Read/Write Error");\
      else if( code==2 ) pgPrint_drawbg(3,32,0xFFFF,0,"FDC Seek Error");      \
      else if( code==3 ) pgPrint_drawbg(3,32,0xFFFF,0,"FDC Over-size Error"); \
      else               pgPrint_drawbg(3,32,0xFFFF,0,"Internal error !");			      \
    }while(0)
//      if     ( code==1 ) printf("FDC Read/Write Error in DRIVE %d:\n", drv+1);\
//      else if( code==2 ) printf("FDC Seek Error in DRIVE %d:\n", drv+1);      \
//      else if( code==3 ) printf("FDC Over-size Error in DRIVE %d:\n", drv+1); \
//      else               printf("Internal error !\n");			      \

#define effective_size( size )						\
	((size)>=0x1000) ? ((size)&0xf000 ) : 				\
		(((size)>=0x100) ? ((size)&0xf00) : ((size)&0x80))


/*	------ ディスクイメージのステータス ------			*/
/*		実際に意味のあるステータスは以下のとおり		*/
/*			STATUS_MA	このセクタの ID は無効		*/
/*			STATUS_DE	ID CRC Error			*/
/*			STATUS_MA_MD	このセクタの DATA は無効	*/
/*			STATUS_DE_DD	DATA CRC Error			*/
/*			STATUS_CM	正常 (DELETED DATA 扱い)	*/
/*			その他		正常				*/

#define	STATUS_NORMAL	(0x00)		/* Normal End			*/
#define	STATUS_CM	(0x10)		/* Control Mark			*/
#define	STATUS_ALIGN	(0x20)		/* Alignment Error		*/
#define	STATUS_EN	(0x30)		/* End of Cylinder		*/
#define STATUS_EC	(0x40)		/* Equipment Check		*/
#define	STATUS_OR	(0x50)		/* Over Run			*/
#define	STATUS_NR	(0x60)		/* Not Ready			*/
#define	STATUS_NW	(0x70)		/* Not Writable			*/
#define	STATUS_UNDEF	(0x80)		/* Another Error		*/
#define	STATUS_TIMEOUT	(0x90)		/* Time out			*/
#define	STATUS_DE	(0xa0)		/* Data Error (ID)		*/
#define	STATUS_DE_DD	(0xb0)		/* Data Error (DATA)		*/
#define	STATUS_ND	(0xc0)		/* No Data			*/
#define	STATUS_BC	(0xd0)		/* Bad Cylinder			*/
#define	STATUS_MA	(0xe0)		/* Missing Address Mark (ID)	*/
#define	STATUS_MA_MD	(0xf0)		/* Missing Address Mark (DATA)	*/




/* FDC の ステータス */

#define FD0_BUSY	(0x01)
#define	FD1_BUSY	(0x02)
#define	FD2_BUSY	(0x04)
#define	FD3_BUSY	(0x08)
#define FDC_BUSY	(0x10)
#define NON_DMA		(0x20)
#define DATA_IO		(0x40)
#define	REQ_MASTER	(0x80)

/* FDC の リザルトステータス */

#define	ST0_US		(0x03)		/* Unit Select			*/
#define	ST0_HD		(0x04)		/* Head Address			*/
#define	ST0_NR		(0x08)		/* Not Ready			*/
#define	ST0_EC		(0x10)		/* Equipment Check		*/
#define	ST0_SE		(0x20)		/* Seek End			*/
#define	ST0_IC		(0xc0)		/* Interrupt Code		*/
#define	ST0_IC_NT	(0x00)		/* 	Normal Terminate	*/
#define	ST0_IC_AT	(0x40)		/* 	Abnormal Terminate	*/
#define	ST0_IC_IC	(0x80)		/* 	Invalid Command		*/
#define	ST0_IC_AI	(0xc0)		/* 	Attention Interrupt	*/

#define	ST1_MA		(0x01)		/* Missing Address Mark		*/
#define	ST1_NW		(0x02)		/* Not Writable			*/
#define	ST1_ND		(0x04)		/* No Data			*/
#define	ST1_OR		(0x10)		/* Over Run			*/
#define	ST1_DE		(0x20)		/* Data Error			*/
#define	ST1_EN		(0x80)		/* End of Cylinder		*/

#define	ST2_MD		(0x01)		/* Missing Address Mark in Data	*/
#define	ST2_BC		(0x02)		/* Bad Cylinder			*/
#define	ST2_SN		(0x04)		/* Scan Not Satisfied		*/
#define	ST2_SH		(0x08)		/* Scan Equal Hit		*/
#define	ST2_NC		(0x10)		/* No Cylinder			*/
#define	ST2_DD		(0x20)		/* Data Error in Data Files	*/
#define	ST2_CM		(0x40)		/* Control Mark			*/

#define	ST3_US		(0x03)		/* Unit Select			*/
#define	ST3_HD		(0x04)		/* Head Address			*/
#define	ST3_TS		(0x08)		/* Two Side			*/
#define	ST3_T0		(0x10)		/* Track 0			*/
#define	ST3_RY		(0x20)		/* Ready			*/
#define	ST3_WP		(0x40)		/* Write Protect		*/
#define	ST3_FT		(0x80)		/* Fault			*/



/* FDC の コマンド */

enum FdcCommand
{
  WAIT			= 0,		/* 処理待ちの状態 */
  READ_DATA,
  READ_DELETED_DATA,
  READ_DIAGNOSTIC,
  READ_ID,
  WRITE_DATA,		/* = 5 */
  WRITE_DELETED_DATA,
  WRITE_ID,
  SCAN_EQUAL,
  SCAN_LOW_OR_EQUAL,
  SCAN_HIGH_OR_EQUAL,	/* = 10 */
  SEEK,
  RECALIBRATE,
  SENSE_INT_STATUS,
  SENSE_DEVICE_STATUS,
  SPECIFY,		/* = 15 */
  INVALID,
  EndofFdcCmd
};
enum FdcPhase
{
  C_PHASE,
  E_PHASE,
  R_PHASE,
  EndofFdcPhase
};








/************************************************************************/
/* fdc デバッグ								*/
/************************************************************************/
/*
 * FDC デバッグ処理は、peach氏により提供されました。
 */
void pc88fdc_break_point(void)
{
    int i;
    for(i = 0; i < NR_BP; i++){
	if (break_point_fdc[i].type != BP_NONE) {
	    fdc_break_flag = TRUE;
	    return;
	}
    }
    fdc_break_flag = FALSE;
}


#ifndef	USE_MONITOR

#define	print_fdc_status(nStatus,nDrive,nTrack,nSector)

#else	/* USE_MONITOR */

void print_fdc_status(int nStatus, int nDrive, int nTrack, int nSector)
{
    static int oDrive = -1;
    static int oTrack[2];
    static int oSector[2];
    static int oStatus;
    char c = ' ';
    int i;
    
    if (fdc_debug_mode == TRUE) {
	if (oDrive < 0 || nStatus != oStatus || nDrive != oDrive ||
	    nTrack != oTrack[nDrive])
	{
	    oStatus = nStatus;
	    oDrive = nDrive;
	    oTrack[oDrive] = nTrack;
	    oSector[oDrive] = nSector;
	    switch (nStatus) {
	    case BP_READ:  c = 'R'; break;
	    case BP_WRITE: c = 'W'; break;
	    case BP_DIAG:  c = 'D'; break;
	    }
	    printf("\n%c D:%d T:%d S:%d", c, nDrive+1, nTrack, nSector+1);
	    fflush(stdout);
	} else if (nSector != oSector[nDrive]){
	    oSector[nDrive] = nSector;
	    printf(",%d", nSector+1);
	    fflush(stdout);
	}
    }
    
    if (fdc_break_flag == TRUE) {
	for (i = 0; i < NR_BP; i++) {
	    if (break_point_fdc[i].type == nStatus &&
	        break_point_fdc[i].drive == nDrive + 1 &&
		break_point_fdc[i].track == nTrack) {
		if (break_point_fdc[i].sector == nSector + 1 ||
		    break_point_fdc[i].sector < 0) {
		    printf( "*** Break at D:%d T:%d S:%d *** ",
			    nDrive + 1, nTrack, nSector + 1);
		    switch (nStatus) {
		    case BP_READ:  printf("( Read )\n"); break;
		    case BP_WRITE: printf("( Write )\n"); break;
		    case BP_DIAG:  printf("( Diag )\n"); break;
	 	    }
		    emu_mode = MONITOR;
		    break;
		}
	    }
	}
    }
 
}
#endif	/* USE_MONITOR */

/************************************************************************/
/* セクタ間を埋める                                                     */
/************************************************************************/
/*
 * READ DIAG のセクタ間のデータを埋める処理は、peach氏により提供されました。
 */
static int fill_sector_gap(int ptr, int drv, Uchar fdc_mf);








/************************************************************************/
/* ドライブの初期化							*/
/************************************************************************/
static	void	fdc_init( void );
void	drive_init( void )
{
  int	i;

  fdc_init();

  for( i=0; i<NR_DRIVE; i++ ){
    drive[ i ].fp     = NULL;
    drive[ i ].sec_nr = -1;
    drive[ i ].empty  = TRUE;
    drive[ i ].filename = NULL;
  }
  disk_ex_drv = 0;
}


/************************************************************************/
/* ドライブのリセット。(現在設定されているイメージにワークを再設定)	*/
/************************************************************************/
void	drive_reset( void )
{
  int	i;

  fdc_init();

  for( i=0; i<NR_DRIVE; i++ ){
    if( drive[ i ].fp ){
      disk_change_image( i, drive[ i ].selected_image );
    }
  }
  disk_ex_drv = 0;
}



/************************************************************************/
/* ドライブ にディスクを挿入する					*/
/*	drv…ドライブ(0/1)  filename…ファイル名  dsk…ディスク番号(0～)*/
/*									*/
/*	1. 指定のディスクイメージファイルを上書きモードでオープンする。	*/
/*	2. 書き込み禁止の時は、リードオンリーでファイルをオープンする。	*/
/*	3. すでに開いているファイルの場合は、情報をコピー		*/
/*	4. fdc.pcn[drv]*2 トラックにシークし、各種ワーク初期化。	*/
/*	エラー時は、ディスクをセットせずに 1 を返す。			*/
/*									*/
/*		・DRIVE の 初期化は完了していること。			*/
/*		・insert の前に eject が行なわれていること。		*/
/************************************************************************/
static	void	disk_insert_error( char *s, int drv )
{
  if( emu_mode != MENU_MAIN  ||
     ( emu_mode == MENU_MAIN && verbose_proc )){
    printf( "\n" "[[[ %-26s ]]]\n", s );
    printf(      "[[[   Eject Disk from drive %d: ]]]\n" "\n", drv+1 );
	pgPrint_drawbg(3,32,0xFFFF,0,s);
  }
  disk_eject( drv );
}


int	disk_insert( int drv, char *filename, int img )
{
  int	i, exit_flag;
  Uchar c[32];
  long	offset;
  int	num, same_image;
  char	*fname;
  int	status;


	/* READ/WRITE でファイルを開く。			*/
	/*	エラーならメッセージを表示し 1を返す。		*/
	/*	成功したら、ファイル名をセットする。		*/

  drive[ drv ].filename = NULL;
  drive[ drv ].file_size = osd_file_size ( filename );
  drive[ drv ].fp = osd_fopen_image( filename, FALSE, &fname, &status );

  if( drive[ drv ].fp == NULL ){
    if( status == FOPEN_NO_MEM ){
      disk_insert_error( "Out of Memory", drv );
    }else{
      disk_insert_error( "Open Error", drv );
    }
    return 1;
  }

  if( status == FOPEN_OK_RO ){
    if( verbose_proc ) printf( " (( Read Only in drive %d: ))\n", drv+1 );
    drive[ drv ].read_only = TRUE;
  }else{
    drive[ drv ].read_only = FALSE;
  }

  drive[ drv ].filename = fname;




	/* 同じファイルをすでに開いていないかチェックする。	*/
	/*	2 ドライブであると限定し、反対のドライブを見る	*/

  if( drive[ drv^1 ].fp ){				/*反対ドライブ使用中 */

    switch( osd_file_cmp( drive[ drv ].filename, drive[ drv^1 ].filename ) ){
    case FILE_CMP_OK:						/* 一致 */
      same_image = TRUE;
      break;
    case FILE_CMP_NOT_OK:					/* 不一致 */
      same_image = FALSE;
      break;
    case FILE_CMP_ERR:						/* エラー */
      disk_insert_error( "Same Check Failed", drv );	return 1;
    default:
      disk_insert_error( "Internal Error", drv );	return 1;
    }

  }else{						/* 反対ドライブ未使用*/
    same_image = FALSE;
  }


	/* 反対ドライブと同じファイルの時は、反対ドライブのワークをコピー */
	/* そうでない時は、ファイルを読んで、各種ワークを設定		  */

  if( same_image ){			/* 反対ドライブと同じファイルの場合 */

    ex_fclose( drive[ drv ].fp );
    drive[ drv ].fp                  = drive[ drv^1 ].fp;
    drive[ drv ].read_only           = drive[ drv^1 ].read_only;
    drive[ drv ].file_size           = drive[ drv^1 ].file_size;
    drive[ drv ].over_image          = drive[ drv^1 ].over_image;
    drive[ drv ].detect_broken_image = drive[ drv^1 ].detect_broken_image;
    drive[ drv ].image_nr            = drive[ drv^1 ].image_nr;
    memcpy( &drive[ drv   ].image,
	    &drive[ drv^1 ].image, sizeof(drive[ drv ].image) );

  }else{				/* 反対ドライブと違うファイルの場合 */

    // drive[ drv ].file_size = osd_file_size( drive[ drv ].filename );
    if( drive[ drv ].file_size == -1 ){
      disk_insert_error( "Access Error(size)", drv );	return 1;
    }

    drive[ drv ].over_image          = FALSE;
    drive[ drv ].detect_broken_image = FALSE;

    num = 0;	offset = 0;
    exit_flag = FALSE;

    while( !exit_flag ){
      switch( fread_header( drive[ drv ].fp, offset, c ) ){

      case 0:					/* イメージ情報取得成功 */
	for(i=0;i<16;i++) drive[ drv ].image[ num ].name[i] = c[i];
	drive[ drv ].image[ num ].name[16] = '\0';
	drive[ drv ].image[ num ].protect  = c[DISK_PROTECT];
	drive[ drv ].image[ num ].type     = c[DISK_TYPE];
	drive[ drv ].image[ num ].size     = read_size_in_header( c );
	offset += read_size_in_header( c );
	num ++;
	if( num >= MAX_NR_IMAGE           &&	/* イメージ数が多い時は中断 */
	    offset < drive[ drv ].file_size ){
	  if( verbose_proc ) 
	    printf(" (( %s : Image number over %d ))\n",filename,MAX_NR_IMAGE);
	  drive[ drv ].over_image = TRUE;
	  exit_flag = TRUE;
	}
	break;

      case 1:					/* これ以上イメージがない */
	exit_flag = TRUE;
	break;

      case 2:					/* このイメージは壊れている */
	if( verbose_proc )
	  printf(" (( %s : Image No. %d Broken ))\n",filename,num+1);
	drive[ drv ].detect_broken_image = TRUE;
	exit_flag = TRUE;
	break;

      case 3:					/* シークエラー発生 */
	printf(" (( %s : Image No. %d Seek error ))\n",filename,num+1);
	drive[ drv ].detect_broken_image = TRUE;
	exit_flag = TRUE;
	break;

      }
    }

    if( num==0 ){ disk_insert_error( "Image broken", drv );  return 1; }
    drive[ drv ].image_nr = num;

  }



	/* disk_top をimg 枚目のディスクイメージの先頭に設定	*/

  if( img >= drive[ drv ].image_nr ){
    disk_insert_error( "Image Number Bad", drv );	return 1;
  }

  disk_change_image( drv, img );



  return 0;
}



/************************************************************************/
/* イメージを変更する。							*/
/*	disk_top をimg 枚目のディスクイメージの先頭に設定し、		*/
/*	pcn*2 トラックの先頭に移動する。				*/
/*	drv…ドライブ(0/1)						*/
/************************************************************************/
static void disk_now_track( int drv, int trk );

int	disk_change_image( int drv, int img )
{
  int	i;



  if( drive[ drv ].fp==NULL ){				/* ファイルが無い時 */
    return 1;
  }
  
  if( img >= drive[ drv ].image_nr ){			/* 指定イメージ無し */
    return 2;
  }

		/* disk_top を計算 */

  drive[ drv ].selected_image = img;
  drive[ drv ].empty          = FALSE;

  drive[ drv ].disk_top = 0;
  for( i=0; i<img; i++ ){
    drive[ drv ].disk_top += drive[ drv ].image[ i ].size;
  }
  drive[ drv ].disk_end = drive[ drv ].disk_top + drive[ drv ].image[img].size;
  drive[ drv ].protect  = drive[ drv ].image[ img ].protect;
  drive[ drv ].type     = drive[ drv ].image[ img ].type;



  if( drive[ drv ].read_only ) drive[ drv ].protect = DISK_PROTECT_TRUE;


		/* pcn*2 トラックの先頭に移動する */

  disk_now_track( drv, fdc.pcn[drv]*2 );

  if (disk_exchange) disk_ex_drv |= 1 << drv;	/* ディスク入れ替えたよん */

  return 0;
}



/************************************************************************/
/* ディスクをイジェクトする						*/
/*	2 ドライブであると限定し、反対のドライブと比較する。		*/
/*	同じであれば、ワークを初期化。違えば、ファイルを閉じる		*/
/*	drv…ドライブ(0/1)						*/
/************************************************************************/
void	disk_eject( int drv )
{
  if( drive[ drv ].fp ){
    if( drive[ drv ].fp != drive[ drv^1 ].fp ){
      ex_fclose( drive[ drv ].fp );
    }
  }
  drive[ drv ].fp = NULL;
  drive[ drv ].sec_nr = -1;
  drive[ drv ].empty  = TRUE;

  if( drive[ drv ].filename ){
    ex_free( drive[ drv ].filename );
    drive[ drv ].filename = NULL;
  }
}

/************************************************************************/
/* ドライブを一時的に空にする／もとに戻す／切替える／どっちの状態か知る	*/
/*	drv…ドライブ(0/1)						*/
/************************************************************************/
void	drive_set_empty( int drv )
{
  drive[ drv ].empty  = TRUE;
}
void	drive_unset_empty( int drv )
{
  drive[ drv ].empty  = FALSE;
}
void	drive_change_empty( int drv )
{
  drive[ drv ].empty  ^= 1;
}
int	drive_check_empty( int drv )
{
  return drive[ drv ].empty;
}










/*======================================================================*/
/* ヘッドをトラックの先頭に移動する					*/
/*	drv…ドライブ(0/1)  trk…トラック番号(0～)			*/
/*									*/
/*	エラーが出た場合は、そのトラックはアンフォーマットになる。	*/
/*======================================================================*/
static int disk_now_sec( int drv );

static	void	disk_now_track( int drv, int trk )
{
  int	error = 0;
  Uchar c[4];
  long	track_top;



	/* シーク可能シリンダのチェック */

  if     ( drive[ drv ].type==DISK_TYPE_2D  && trk>=84  ) trk =  83;
  else if( drive[ drv ].type==DISK_TYPE_2DD && trk>=164 ) trk = 163;
  else if( drive[ drv ].type==DISK_TYPE_2HD && trk>=158 ) trk = 157;
  else if( trk>=164 ) trk = 163; /* ここまでのばすと 2DD/2HD に対応できる */
						        /* thanks peach ! */

	/* ワーク設定 & 初期化 */

  drive[ drv ].track     = trk;
  drive[ drv ].sec       = 0;


	/* トラックのインデックスで指定されたファイル位置を取得 */

  if( ex_fseek( drive[ drv ].fp,
	     drive[ drv ].disk_top + DISK_TRACK + trk*4,  SEEK_SET )==0 ){
    if(ex_fread( c, sizeof(Uchar), 4, drive[ drv ].fp )==4 ){

	/* トラックおよび、先頭セクタの位置を設定   */
	/* そのセクタのセクタ情報および、セクタ数を得る */

      track_top = (long)c[0]+((long)c[1]<<8)+((long)c[2]<<16)+((long)c[3]<<24);
      if( track_top!=0 ){
	drive[ drv ].track_top =
	drive[ drv ].sec_pos   = drive[ drv ].disk_top + track_top;
	drive[ drv ].sec_nr    = disk_now_sec( drv );
      }else{
	drive[ drv ].track_top =
	drive[ drv ].sec_pos   = drive[ drv ].disk_top;
	drive[ drv ].sec_nr    = -1;
      }
    }
    else error = 1;
  } else error = 2;

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
  }

	/* エラー時は、アンフォーマット(再フォーマット不能)にして、戻る */

  if( error ){
    drive[ drv ].track_top =
    drive[ drv ].sec_pos   = drive[ drv ].disk_top;
    drive[ drv ].sec_nr    = -1;
  }
  return;
}






/*======================================================================*/
/* 指定されたディスクの現在のセクタの情報を読みとる			*/
/*	drv…ドライブ(0/1)						*/
/*									*/
/*	エラー時は、そのセクタのは ID CRC Error エラーに設定する。	*/
/*	返り値は、そのセクタの、「セクタ数(DISK_SEC_NR)」の値		*/
/*======================================================================*/
static	int	disk_now_sec( int drv )
{
  int	error = 0;
  Uchar	c[16];

	/* ファイル位置 sec_pos の ID情報 を読み、セクタ数を返す */

  if( ex_fseek( drive[ drv ].fp,  drive[ drv ].sec_pos,  SEEK_SET )==0 ){
    if(ex_fread( c, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
      sec_buf.c       = c[DISK_C];
      sec_buf.h       = c[DISK_H];
      sec_buf.r       = c[DISK_R];
      sec_buf.n       = c[DISK_N];
      sec_buf.density = c[DISK_DENSITY];
      sec_buf.deleted = c[DISK_DELETED];
      sec_buf.status  = c[DISK_STATUS];
      sec_buf.sec_nr  = c[DISK_SEC_NR] + (int)c[DISK_SEC_NR+1]*256;
      sec_buf.size    = c[DISK_SEC_SZ] + (int)c[DISK_SEC_SZ+1]*256;
      if( sec_buf.status==STATUS_CM ){
	sec_buf.deleted = DISK_DELETED_TRUE;
	sec_buf.status  = STATUS_NORMAL;
      }
    }
    else error = 1;
  } else error = 2;

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
  }



	/* 失敗したら、ID CRC Error にし、0 (==unformat) を返す */

  if( error ){
    sec_buf.sec_nr  = 0;
    sec_buf.status  = STATUS_DE;
  }

  return ( sec_buf.sec_nr );
}


/*======================================================================*/
/* 指定されたディスクの次のセクタの情報を読みとる			*/
/*	drv…ドライブ(0/1)						*/
/*======================================================================*/
static	void	disk_next_sec( int drv )
{
  int	overwrite_id;

	/* アンフォーマット時は、なにもしない */

  if( disk_unformat( drv ) ) return;


	/* sec_top を次のセクタに。最終セクタの時はトラック先頭に */

			/* ミックスセクタ作成時に上書きされた ID の数 */
			/* この辺が正確にチェックできない。どうしよう */

  if( sec_buf.size == 0x80 ||		/* sec_buf.sizeが 0x80,0x100,0x200 */
     (sec_buf.size & 0xff) == 0 ){	/* 0x400,0x800,0x1000 の場合(正常) */
    overwrite_id = 0;
  }else{				/* それ以外は、ミックスセクタかも  */
    overwrite_id = ( sec_buf.size - ( 128 << (sec_buf.n & 7)) ) / SZ_DISK_ID;
    if( overwrite_id < 0 ) overwrite_id = 0;
  }


  drive[ drv ].sec += ( 1 + overwrite_id );
  if( drive[ drv ].sec < drive[ drv ].sec_nr ){

    drive[ drv ].sec_pos += (sec_buf.size + SZ_DISK_ID);

  }else{

    drive[ drv ].sec = 0;
    drive[ drv ].sec_pos = drive[ drv ].track_top;

  }

	/* sec_pos の セクタID情報 を読む */

  disk_now_sec( drv );
}




















/*----------------------------------------------------------------------*/
/* READ ID								*/
/*	現在のセクタ ID が正常ならば、その内容を返す。			*/
/*	異常ならば、次のセクタ ID を読んで、やりなおし。		*/
/*	2周分チェックしても正常なセクタが無い時は、エラーで終了。	*/
/*	いずれの場合も、戻る前に、次のセクタを読んでおく。		*/
/*	ワーク）正常時	fdc.c, h, r, n, st0, st1, st2			*/
/*		異常時	fdc.st0, st1, st2				*/
/*----------------------------------------------------------------------*/
static	int	disk_read_id( void )
{
  int	index_cnt, exist_am;
  int	drv = fdc.us;


	/* ドライブ 2/3 は未サポート */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    fdc.c=fdc.h=fdc.r=fdc.n=0xff;
    return 1;
  }

	/* ディスクが無い時は、いつまでたっても終らない */

  if( disk_not_exist( drv ) ) return 0;


	/* 現在のヘッド位置が指定と違う時は、ヘッド移動 */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }


	/* アンフォーマットの時は、終了 */

  if( disk_unformat( drv ) ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    fdc.c=fdc.h=fdc.r=fdc.n=0xff;
    return 1;
  }


	/* ID CRC エラーのない最初のセクタを得る。 */

  index_cnt = 0;				/* インデックスホール検出回数*/
  exist_am  = FALSE;				/* AMが 1度でも見つかったら真*/

  while( 1 ){	/*===== 見つかるか、インデックスホール2回検出までループ =====*/

    if( index_cnt >= 2 ){		/* インデックスホール2回検出した */
      if( exist_am ){
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_ND;
	fdc.st2 = 0;
	if( fdc.c!=fdc.pcn[drv] ) fdc.st2 |= ST2_NC;
	if( fdc.c==0xff )         fdc.st2 |= ST2_BC;
      }else{
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_MA;
	fdc.st2 = 0;
      }
      fdc.c=fdc.h=fdc.r=fdc.n=0xff;
      return 1;				/*	→ 失敗 		*/
    }
    if( drive[ drv ].sec==0 ) index_cnt ++;


    if( sector_density_mismatch()   ||		/* このセクタには AM がない */
        sec_buf.status == STATUS_MA ){

      /* 残念。次のセクタの検索に進む */

    }else{					/* このセクタには AM がある */
      exist_am = TRUE;

      if( sec_buf.status == STATUS_DE ){		/* ID CRC Errの時 */

	/* 残念。次のセクタの検索に進む */

      }else{						/* ID CRC 正常の時 */
	/* よし、セクタ情報を ST0～2 に設定 */
	break;
      }
    }

    disk_next_sec( drv );		/* 次のセクタの検索	*/
  }		/*===========================================================*/



		/* ID を設定する */

  fdc.c   = sec_buf.c;
  fdc.h   = sec_buf.h;
  fdc.r   = sec_buf.r;
  fdc.n   = sec_buf.n;
  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  disk_next_sec( drv );
  return 1;
}


/*----------------------------------------------------------------------*/
/* READ DATA								*/
/* READ DELELTED DATA							*/
/*	現在のセクタから順に、ID を検索。指定されたIDが見つかれば、	*/
/*	データをバッファに格納。sec_top は次のセクタの先頭をさす。	*/
/*	トラック2周分 ID を検索しても指定したIDが無い時はエラー。この時	*/
/*	sec_top はトラックの先頭を指す。				*/
/*	ID の N と size が異なる場合、size >= N なら、余りのデータは	*/
/*	無視される。size < N なら、足りない部分は適当に埋められる。	*/
/*	この時、ID ステータスは必ず、DATA CRC エラーにする。		*/
/*	ワーク）正常時	fdc.st0, st1, st2 / data_buf			*/
/*		異常時	fdc.st0, st1, st2				*/
/*----------------------------------------------------------------------*/
	/* * * * * * * * * * * * * * * */
	/* STEP 1 : 指定の ID を検索   */
	/* * * * * * * * * * * * * * * */
static	int	disk_read_data__id( void )
{
  int	index_cnt, exist_am;
  int	drv = fdc.us;


	/* ドライブ 2/3 は未サポート */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }

	/* ディスクが無い時は、いつまでたっても終らない */

  if( disk_not_exist( drv ) ) return 0;


	/* 現在のヘッド位置が指定と違う時は、ヘッド移動 */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }


	/* アンフォーマットの時は、終了 */

  if( disk_unformat( drv ) ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


	/* IDR と一致するセクタを探す。		*/

  index_cnt = 0;				/* インデックスホール検出回数*/
  exist_am  = FALSE;				/* AMが 1度でも見つかったら真*/

  while( 1 ){	/*===== 見つかるか、インデックスホール2回検出までループ =====*/

    if( index_cnt >= 2 ){		/* インデックスホール2回検出した */
      if( exist_am ){
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_ND;
	fdc.st2 = 0;
	if( fdc.c!=fdc.pcn[drv] ) fdc.st2 |= ST2_NC;
	if( fdc.c==0xff )         fdc.st2 |= ST2_BC;
      }else{
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_MA;
	fdc.st2 = 0;
      }
      return 1;				/*	→ 失敗 		*/
    }
    if( drive[ drv ].sec==0 ) index_cnt ++;


    if( sector_density_mismatch()   ||		/* このセクタには AM がない */
        sec_buf.status == STATUS_MA ){

      /* 残念。次のセクタの検索に進む */

    }else{					/* このセクタには AM がある */
      exist_am = TRUE;

      if( idr_match() ){			/* IDR も一致した 	*/

	if( sec_buf.status == STATUS_DE ){		/* ID CRC Errの時 */
	  fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	  fdc.st1 = ST1_DE;
	  fdc.st2 = 0;
	  disk_next_sec( drv );
	  return 1;					/*	→ 失敗 */
	}else{						/* ID CRC 正常の時 */
	  /* よし、データをリードだ */
	  break;					/*	→ 成功 */
	}

      }else{					/*  IDR 一致しない	*/

	/* 残念。次のセクタの検索に進む */

      }
    }

    disk_next_sec( drv );		/* 次のセクタの検索	*/
  }		/*===========================================================*/



		/* DAM/DDAM を検索 */

  if( sec_buf.status == STATUS_MA_MD ){		/* ---- DAM/DDAM がない */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = ST2_MD;
    disk_next_sec( drv );
    return 1;
  }


		/* DELETED かどうかを ST2 にセットし一旦終了。ST0 は NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 2 : そのセクタをリード */
	/* * * * * * * * * * * * * * * */
static	int	disk_read_data__data( void )
{
  int	read_size, size, ptr, error;
  int	drv = fdc.us;

  print_fdc_status(BP_READ, drv, drive[drv].track, drive[drv].sec);

		/* DATA CRC エラーの場合、STATUS を設定 */

  if( sec_buf.status==STATUS_DE_DD ){
    fdc.st0 |= ST0_IC_AT;
    fdc.st1 |= ST1_DE;
    fdc.st2 |= ST2_DD;
  }


		/* DATA 部分を読む */

  read_size = 128 << (fdc.n & 7);		/* 読み込みサイズ       */
  ptr       = 0;				/* 書き込み位置		*/

  while( read_size > 0 ){	/*========== 指定サイズ分読み続ける =========*/

    size = effective_size( sec_buf.size );
    if( ex_fseek( drive[ drv ].fp,
	       drive[ drv ].sec_pos + SZ_DISK_ID,  SEEK_SET )==0 ){
      if(ex_fread( &data_buf[ptr], sizeof(Uchar),
		 size, drive[ drv ].fp )==(size_t)size ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    if( error ){
      printf_system_error( error );
      fdc.st0 |= ST0_IC_AT;
      fdc.st1 |= ST1_DE;
      fdc.st2 |= ST2_DD;
    }

    ptr       += size;
    read_size -= size;
    if( read_size <= 0 ) break;


    fdc.st0 |= ST0_IC_AT;			/* 次のセクタに跨った */
    fdc.st1 |= ST1_DE;
    fdc.st2 |= ST2_DD;
    if( verbose_fdc )
      printf("FDC Read Data : Sector OverRead in track %d (DRIVE %d:)\n",
	      drive[drv].track, drv+1);


	/* セクタ間を埋める。ここには、   				*/
	/* DATA CRC, GAP3, ID SYNC, AM, ID, GAP2 などが入る(内容未作成)	*/
		    /* CRC  GAP3  SYNC   AM    ID  CRC GAP2 */
    if( fdc.mf ) size = 2 + 0x36 + 12 + (3+1) + 4 + 2 + 22;
    else         size = 2 + 0x2a +  6 + (1+1) + 4 + 2 + 11;

    ptr       += size;
    read_size -= size;

    disk_next_sec( drv );
  }			/*===================================================*/

  disk_next_sec( drv );
  return 1;
}




/*----------------------------------------------------------------------*/
/* READ DIAGNOSTIC							*/
/*	track_topから順に、ID を検索。AM のあるID がみつかれば、	*/
/*	IDR の一致不一致とは無関係に、データをバッファに格納。		*/
/*	指定の N が、ID の N よりも大きければ、引続き次のセクタのデータ	*/
/*	もバッファに格納。						*/
/*	それ以外は基本的に、READ DATA に準じる				*/
/*----------------------------------------------------------------------*/
	/* * * * * * * * * * * * * * * */
	/* STEP 1 : 指定の ID を検索   */
	/* * * * * * * * * * * * * * * */
static	int	disk_read_diagnostic__id( void )
{
  int	index_cnt;
  int	drv = fdc.us;


	/* ドライブ 2/3 は未サポート */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }

	/* ディスクが無い時は、いつまでたっても終らない */

  if( disk_not_exist( drv ) ) return 0;


	/* ヘッド移動(先頭セクタに移動する) */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }else{
    disk_now_track( drv,   drive[drv].track );
  }


	/* アンフォーマットの時は、終了 */

  if( disk_unformat( drv ) ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


	/* ID の有効な、最初のセクタを得る。 */

  index_cnt = 0;				/*インデックスホール検出回数 */

  while( 1 ){	/*===== 見つかるか、インデックスホール2回検出までループ =====*/

    if( index_cnt >= 2 ){		/* インデックスホール2回検出した */
      fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
      fdc.st1 = ST1_MA;
      fdc.st2 = 0;
      return 1;				/*	→ 失敗 		*/
    }
    if( drive[ drv ].sec==0 ) index_cnt ++;


    if( sector_density_mismatch()   ||		/* このセクタには AM がない */
        sec_buf.status == STATUS_MA ){

      /* 残念。次のセクタの検索に進む */

    }else{					/* このセクタには AM がある */

      if( sec_buf.status == STATUS_MA_MD ){		/* DATA mark なし */

	return 0;					/* → ハングする */

      }else{						/* DATA mark あり */
	/* よし、データをリードダイアグノスティックだ */
	break;
      }

    }

    disk_next_sec( drv );		/* 次のセクタの検索	*/
  }		/*===========================================================*/



		/* 一旦戻る。ST0 は NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 2 : データをベタ読み   */
	/* * * * * * * * * * * * * * * */
static	int	disk_read_diagnostic__data( void )
{
  int	diag_size, size, ptr, error;
  int	drv = fdc.us;

  print_fdc_status(BP_DIAG, drv, drive[drv].track, drive[drv].sec);

		/* とりあえず、STATUS を設定 */

  if( sec_buf.status==STATUS_DE ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_DE;
    fdc.st2 = ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  else
  if( sec_buf.status==STATUS_DE_DD ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_DE;
    fdc.st2 = ST2_DD | ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  else{
    fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  if( !idr_match() ){
    fdc.st0 |= ST0_IC_AT;
    fdc.st1 |= ST1_ND;
  }


		/* DATA 部分を読む */

  diag_size = 128 << (fdc.n & 7);		/* 読み込みサイズ       */
  ptr       = 0;				/* 書き込み位置		*/

  if((sec_buf.size==0x80 && diag_size!=0x80) ||	 /* セクタのN と、読みだし*/
     (sec_buf.size & 0xff00) != diag_size    ){	 /* のN が違う時は、      */
    fdc.st0 |= ST0_IC_AT;			 /* DATA CRC err          */
    fdc.st1 |= ST1_DE;
    fdc.st2 |= ST2_DD;
  }

  while( diag_size > 0 ){	/*========== 指定サイズ分読み続ける =========*/

    size = effective_size( sec_buf.size );
    if( ex_fseek( drive[ drv ].fp,
	       drive[ drv ].sec_pos + SZ_DISK_ID,  SEEK_SET )==0 ){
      if(ex_fread( &data_buf[ptr], sizeof(Uchar),
		 size, drive[ drv ].fp )==(size_t)size ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    if( error ){
      printf_system_error( error );
      fdc.st0 |= ST0_IC_AT;
      fdc.st1 |= ST1_DE;
      fdc.st2 |= ST2_DD;
    }

    ptr       += size;
    diag_size -= size;
    if( diag_size <= 0 ) break;

    fdc.st0 |= ST0_IC_AT;			/* 次のセクタに跨った */
    fdc.st1 |= ST1_DE;
    fdc.st2 |= ST2_DD;

	/* セクタ間を埋める。ここには、   				*/
	/* DATA CRC, GAP3, ID SYNC, AM, ID, GAP2 などが入る		*/

#if 0	/* 旧バージョンでは、セクタ間のデータ作成なし */
		    /* CRC  GAP3  SYNC   AM    ID  CRC GAP2 */
    if( fdc.mf ) size = 2 + 0x36 + 12 + (3+1) + 4 + 2 + 22;
    else         size = 2 + 0x2a +  6 + (1+1) + 4 + 2 + 11;

    ptr       += size;
    diag_size -= size;

    disk_next_sec( drv );

#else	/* peach氏より、セクタ間データ生成処理が提供されました */

    size = fill_sector_gap(ptr, drv, fdc.mf);
    if (size < -1) return 1;

    ptr       += size;
    diag_size -= size;
#endif
  }			/*===================================================*/

  disk_next_sec( drv );
  return 1;
}






/*----------------------------------------------------------------------*/
/* WRITE ID								*/
/*	トラックの先頭から、ID を書き込んでいく。			*/
/*	ワーク）正常時	fdc.st0, st1, st2 / data_buf			*/
/*		異常時	fdc.st0, st1, st2				*/
/*----------------------------------------------------------------------*/
	/* * * * * * * * * * * * * * * */
	/* STEP 1 : ディスクをチェック */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_id__check( void )
{
  int	drv = fdc.us;


	/* ドライブ 2/3 は未サポート */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }

	/* ディスクが無い時は、いつまでたっても終らない */

  if( disk_not_exist( drv ) ) return 0;


	/* ライトプロテクト時は、速攻で終る */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_NW;
    fdc.st2 = 0;
    return 1;
  }

  
	/* 一旦終了。ST0 は NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 2 : フォーマットする   */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_id__write( void )
{
  int	size, error, i, j, id_ptr;
  long	format_pos;
  char	id[SZ_DISK_ID],	data[128];
  int	drv = fdc.us;
  char *fname;
  int status;

	/* 処理の最中に、ディスクが交換されたりした時の処理 */

  if( disk_not_exist( drv ) ) return 0;			/* ディスク無し */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){	/* 書き込み禁止 */
    fdc.st0 = ST0_IC_AT | ST0_EC | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }


	/* ヘッド移動(先頭セクタに移動する) */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }else{
    disk_now_track( drv,   drive[drv].track );
  }


	/* フォーマット不能の時は、なにも無かったかのように終了 */
	/* ^^^^^^^^^^^^^^^^ ---> DISKイメージ自体が壊れている時 */

  if( disk_unformatable( drv ) ){
    if( verbose_fdc )
      printf("FDC Write ID : Track %d missing (DRIVE %d:)\n",
	     drive[drv].track, drv+1);
    fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }


	/* フォーマットデータの準備 */

  for( size=0; size<128; size++ ) data[size] = fdc.d;


	/*        フォーマットパラメータの解析       */
	/* トラック1周を越えないか判定。越えた時は、 */
	/* 最後の1周に収まる分のみを WRITE ID する。 */

  id_ptr = 0;
  {
    int	SZ_BYTE = (drive[ drv ].type==DISK_TYPE_2HD) ? 10400 : 6250;
    int	SZ_GAP0 = (fdc.mf) ? (80+12+3+1    +50) : (40+6+1+1+    50);
    int	SZ_AM   = (fdc.mf) ? (   12+3+1+4+2+22) : (   6+1+1+4+2+11);
    int	SZ_DAM  = (fdc.mf) ? (   12+3+1+0+2+ 0) : (   6+1+1+0+2+ 0);
    int	max_sec;
    max_sec = (SZ_BYTE-SZ_GAP0)/( SZ_AM+SZ_DAM + 128*(1<<(fdc.n&7))+ fdc.gpl );
    if( fdc.sc > max_sec ){

      if( verbose_fdc )
        printf("FDC Write ID : Over Sector %d -> fixed %d (DRIVE %d:)\n",
	       fdc.sc, max_sec, drv+1 );

      id_ptr = (fdc.sc - max_sec) * 4;
      fdc.sc = max_sec;

    }
  }

	/* ID / DATA をファイルに書き込む */

  format_pos = drive[ drv ].track_top;
  error = 0;

  if( fdc.sc==0 ){				/* アンフォーマットを作成 */

    for( i=0; i<SZ_DISK_ID; i++ ) id[ i ] = 0;

    if( format_pos + 16 <= drive[drv].disk_end ){
    if( ex_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
      if(ex_fwrite( id, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    } else error = 3;

  }else for( i=0; i<fdc.sc; i++ ){		/* フォーマットを作成 */

    id[ DISK_C ] = data_buf[ id_ptr++ ];
    id[ DISK_H ] = data_buf[ id_ptr++ ];
    id[ DISK_R ] = data_buf[ id_ptr++ ];
    id[ DISK_N ] = data_buf[ id_ptr++ ];
    id[ DISK_SEC_NR   ] = (fdc.sc    ) & 0xff;
    id[ DISK_SEC_NR+1 ] = (fdc.sc>>8 ) & 0xff;
    id[ DISK_DENSITY ] = (fdc.mf) ? DISK_DENSITY_DOUBLE : DISK_DENSITY_SINGLE;
    id[ DISK_DELETED ] = DISK_DELETED_FALSE;
    id[ DISK_STATUS ] = STATUS_NORMAL;
    id[ DISK_RESERVED+0 ] = 0;
    id[ DISK_RESERVED+1 ] = 0;
    id[ DISK_RESERVED+2 ] = 0;
    id[ DISK_RESERVED+3 ] = 0;
    id[ DISK_RESERVED+4 ] = 0;
    id[ DISK_SEC_SZ   ] = ( 128*(1<<(fdc.n&7))      ) & 0xff;
    id[ DISK_SEC_SZ+1 ] = ( 128*(1<<(fdc.n&7)) >> 8 ) & 0xff;

    if( verbose_fdc )
      if( id[ DISK_N ] != fdc.n )
        printf("FDC Write ID : Mix Sector in track %d (DRIVE %d:)\n",
	       drive[drv].track, drv+1);

    if( format_pos + 16 <= drive[drv].disk_end ){
    if( ex_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
      if(ex_fwrite( id, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
	format_pos += 16;

	for( j=0; j<(1<<(fdc.n&7)); j++ ){

	  if( format_pos + 128 <= drive[drv].disk_end ){
	  if( ex_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
	    if(ex_fwrite( data, sizeof(Uchar), 128, drive[ drv ].fp )==128 ){
	      format_pos += 128;
	    }
	    else{ error = 1; break; }
	  } else{ error = 2; break; }
	  } else{ error = 3; break; }

	}

      }
      else error = 1;
    } else error = 2;
    } else error = 3;

  }

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
  }

  fflush( drive[ drv ].fp );


	/* 途中、システムのエラーが起こっても正常終了する */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;


  disk_now_track( drv,   drive[drv].track );
  return 1;

}
/*----------------------------------------------------------------------*/
/* WRITE DATA								*/
/* WRITE DELELTED DATA							*/
/*	現在のセクタから順に、ID を検索。指定されたIDが見つかれば、	*/
/*	バッファのデータを書き込む。sec_top は次のセクタの先頭をさす。	*/
/*	トラック2周分 ID を検索しても指定したIDが無い時はエラー。この時	*/
/*	sec_top はトラックの先頭を指す。				*/
/*	ID の N と size が異なる場合、size >= N なら、余りの部分は更新	*/
/*	されない。size < N なら、次のセクタにまたがってデータを書き込む */
/*	この時そのセクタの ID ステータスは異常に設定される。		*/
/*	ワーク）正常時	fdc.st0, st1, st2 / data_buf			*/
/*		異常時	fdc.st0, st1, st2				*/
/*----------------------------------------------------------------------*/
	/* * * * * * * * * * * * * * * */
	/* STEP 1 : ディスクをチェック */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_data__check( void )
{
  int	drv = fdc.us;

	/* ドライブ 2/3 は未サポート */

  if( drv >= NR_DRIVE || fdc.status & (1<<drv) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    return 1;
  }

	/* ディスクが無い時は、いつまでたっても終らない */

  if( disk_not_exist( drv ) ) return 0;


	/* ライトプロテクト時は、速攻で終る */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_NW;
    fdc.st2 = 0;
    return 1;
  }

  
	/* 一旦終了。ST0 は NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 2 : 指定の ID を検索   */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_data__search( void )
{
  int	index_cnt, exist_am;
  int	drv = fdc.us;


	/* 処理の最中に、ディスクが交換されたりした時の処理 */

  if( disk_not_exist( drv ) ) return 0;			/* ディスク無し */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){	/* 書き込み禁止 */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


	/* 現在のヘッド位置が指定と違う時は、ヘッド移動 */

  if( ( drive[drv].track & 1 ) != fdc.hd ){
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
  }


	/* アンフォーマット 及び、フォーマット不能の時は、終了 */

  if( disk_unformat( drv ) || disk_unformatable( drv ) ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


	/* IDR と一致するセクタを探す。		*/

  index_cnt = 0;				/* インデックスホール検出回数*/
  exist_am  = FALSE;				/* AMが 1度でも見つかったら真*/

  while( 1 ){	/*===== 見つかるか、インデックスホール2回検出までループ =====*/

    if( index_cnt >= 2 ){		/* インデックスホール2回検出した */
      if( exist_am ){
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_ND;
	fdc.st2 = 0;
	if( fdc.c!=fdc.pcn[drv] ) fdc.st2 |= ST2_NC;
	if( fdc.c==0xff )         fdc.st2 |= ST2_BC;
      }else{
	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = ST1_MA;
	fdc.st2 = 0;
      }
      return 1;				/*	→ 失敗 		*/
    }
    if( drive[ drv ].sec==0 ) index_cnt ++;


    if( sector_density_mismatch()   ||		/* このセクタには AM がない */
        sec_buf.status == STATUS_MA ){

      /* 残念。次のセクタの検索に進む */

    }else{					/* このセクタには AM がある */
      exist_am = TRUE;

      if( idr_match() ){			/* IDR も一致した 	*/

	if( sec_buf.status == STATUS_DE ){		/* ID CRC Errの時 */
	  fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	  fdc.st1 = ST1_DE;
	  fdc.st2 = 0;
	  disk_next_sec( drv );
	  return 1;					/*	→ 失敗 */
	}else{						/* ID CRC 正常の時 */
	  /* よし、データをライトだ */
	  break;					/*	→ 成功 */
	}

      }else{					/*  IDR 一致しない	*/

	/* 残念。次のセクタの検索に進む */

      }
    }

    disk_next_sec( drv );		/* 次のセクタの検索	*/
  }		/*===========================================================*/



		/* ID 無事発見。一旦終了。 ST0 は NT */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}
	/* * * * * * * * * * * * * * * */
	/* STEP 3 : そのセクタにライト */
	/* * * * * * * * * * * * * * * */
static	int	disk_write_data__write( void )
{
  long	id_pos, write_pos;
  int	write_size, total_size, size, ptr, error;
  int	drv = fdc.us;
  unsigned char	c[2];
  char *fname;
  int status;

  print_fdc_status(BP_WRITE, drv, drive[drv].track, drive[drv].sec);

	/* 処理の最中に、ディスクが交換されたりした時の処理 */

  if( disk_not_exist( drv ) ) return 0;			/* ディスク無し */

  if( drive[ drv ].protect==DISK_PROTECT_TRUE ){	/* 書き込み禁止 */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }

  if( disk_unformat( drv ) || disk_unformatable( drv ) ){/* アンフォーマット */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;		/*  フォーマット不能 */
    fdc.st1 = ST1_MA;
    fdc.st2 = 0;
    return 1;
  }


		/* DATA 部分を書く */

  id_pos     = drive[ drv ].sec_pos;		/* IDを書き戻す際の情報   */

  write_size = 128 << (fdc.n & 7);		/* 書き込みサイズ         */
  ptr        = 0;				/* 読み出しのポインタ     */
  write_pos  = drive[drv].sec_pos + SZ_DISK_ID;	/* 書き込みのファイル位置 */
  total_size = sec_buf.size;			/* セクタのデータ部サイズ */

  while( write_size > 0 ){	/*========== 指定サイズ分書き続ける =========*/

    size = effective_size( sec_buf.size );
    if( write_pos + size <= drive[drv].disk_end ){
    if( ex_fseek( drive[ drv ].fp,
	       write_pos,  SEEK_SET )==0 ){
      if(ex_fwrite( &data_buf[ptr], sizeof(Uchar),
		 size, drive[ drv ].fp )==(size_t)size ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    } else error = 3;
    if( error ){
      printf_system_error( error );
    }

    ptr        += size;
    write_size -= size;

    if( write_size<=0 ) break;

    disk_next_sec( drv );			/* 次のセクタに跨った */

    write_pos  += size;
    total_size += (sec_buf.size + SZ_DISK_ID);

    if( verbose_fdc )
      printf("FDC Write Data : Sector OverWrite in track %d (DRIVE %d:)\n",
	     drive[drv].track, drv+1);

  }			/*===================================================*/


		/* ID 部を更新する。*/

  if( fdc.mf && fdc.n==0 ){
    c[0] = DISK_DELETED_FALSE;
    c[1] = STATUS_MA_MD;
  }else{
    if( fdc.command==WRITE_DATA ){
      c[0] = DISK_DELETED_FALSE;
      c[1] = STATUS_NORMAL;
    }else{
      c[0] = DISK_DELETED_TRUE;
      c[1] = STATUS_CM;
    }
  }
  if( ex_fseek( drive[ drv ].fp,			/* ID の、DAM/DDAMを更新 */
	     id_pos + DISK_DELETED,  SEEK_SET )==0 ){
    if(ex_fwrite( &c[0], sizeof(Uchar), 2, drive[ drv ].fp )==2 ){
      error = 0;
    }
    else error = 1;
  } else error = 2;
  if( error ){
    printf_system_error( error );
  }


  c[0] = ( total_size >>  0  ) & 0xff;
  c[1] = ( total_size >>  8  ) & 0xff;
  if( ex_fseek( drive[ drv ].fp,			/* ID の、セクタサイズを更新 */
	     id_pos + DISK_SEC_SZ,  SEEK_SET )==0 ){
    if(ex_fwrite( &c[0], sizeof(Uchar), 2, drive[ drv ].fp )==2 ){
      error = 0;
    }
    else error = 1;
  } else error = 2;
  if( error ){
    printf_system_error( error );
  }


//  fflush( drive[ drv ].fp );


	/* 途中、システムのエラーが起こっても正常終了する */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = ( (fdc.command==WRITE_DELETED_DATA)? ST2_CM: 0 );


  disk_next_sec( drv );
  return 1;
}










/************************************************************************/
/* FDC の初期化								*/
/************************************************************************/
static	void	fdc_init( void )
{
  int	i;

  fdc.status  = 0 | REQ_MASTER;
  fdc.read    = 0xff;
  fdc.write   = 0xff;
  fdc.TC      = FALSE;

  fdc.command = WAIT;

  for( i=0; i<MAX_DRIVE; i++ ){
    fdc.seek_wait[ i ] = -1;
    fdc.ncn[ i ]  = 0;
    fdc.pcn[ i ]  = 0;
  }
}



/************************************************************************/
/* CPUが FDC にアクセスした時に呼ぶ関数郡				*/
/*	void	fdc_write( byte data )	……… OUT A,(0FBH)		*/
/*	byte	fdc_read( void )	……… IN  A,(0FBH)		*/
/*	byte	fdc_status( void )	……… IN  A,(0FAH)		*/
/*	void	fdc_TC( void )		……… IN  A,(0F8H)		*/
/************************************************************************/
void	fdc_write( byte data )
{
  if( (fdc.status & DATA_IO)==0 ){
    fdc.status &= ~REQ_MASTER;
    fdc.write   = data;
  }
}

byte	fdc_read( void )
{
  if( (fdc.status & DATA_IO) ){
    fdc.status &= ~REQ_MASTER;
    return fdc.read;
  }else{
    return 0xff;
  }
}

byte	fdc_status( void )
{
  return fdc.status;
}

void	fdc_TC( void )
{
  fdc.TC = TRUE;
}

/************************************************************************/
/* FDC から、CPUへの割り込み通知関数 (マクロ)				*/
/************************************************************************/
#define	fdc_occur_interrupt()	FDC_flag = TRUE
#define	fdc_cancel_interrupt()	FDC_flag = FALSE



/************************************************************************/
/* E-PHASE 正常終了時に、次の CHRN を指す				*/
/*	2D/2DD の資料がない……… ;_;					*/
/************************************************************************/
static	int	fdc_update_next_chrn( void )
{
  if( fdc.mt==0 ){		/* マルチトラック処理をしない時 */

    if( fdc.r == fdc.eot ){		/* 最終セクタなら、	*/
      fdc.c ++;				/* C+=1, R=1 にする。	*/
      fdc.r = 1;			/* 返り値は エラー	*/
      return 1;				/*   (TC 無に備えて)	*/
    }else{
      fdc.r ++;				/* 最終でなければ、R+=1	*/
      return 0;				/* 返り値は 正常	*/
    }

  }else{			/* マルチトラック処理の時 */

    if( fdc.hd==0 ){		/*   表面処理時 */

      if( fdc.r == fdc.eot ){		/* 最終セクタなら、	*/
	fdc.hd = 1;			/* 裏面に切替えて、	*/
	fdc.h ^= 1;			/* H 反転、R = 1	*/
	fdc.r  = 1;			/* 返り値は 正常	*/
	return 0;
      }else{
	fdc.r ++;			/* 最終でなければ、R+=1	*/
	return 0;			/* 返り値は 正常	*/
      }

    }else{			/*   裏面処理時 */

      if( fdc.r == fdc.eot ){		/* 最終セクタなら、	*/
	fdc.h ^= 1;			/* H 反転、		*/
	fdc.c ++;			/* C+=1, R=1 にする。	*/
	fdc.r = 1;			/* 返り値は エラー	*/
	return 1;			/*   (TC 無に備えて)	*/
      }else{
	fdc.r ++;			/* 最終でなければ、R+=1	*/
	return 0;			/* 返り値は 正常	*/
      }
    }
  }
}



/************************************************************************/
/* FDC の 処理メイン							*/
/*	引数は 前回にこの関数を呼んだ時からの経過時間( 4MHzステートで )	*/
/************************************************************************/

#define	RD	(4)		/*  1us (4MHz) */
#define	WR	(4)		/*  1us (4MHz) */
#define	BYTE	(128)		/* 32us (4MHz) */
#define	ID	(  22*BYTE)	/*     22 byte */
#define	DATA	( 296*BYTE)	/* 256+40 byte */
#define	TRACK	(6250*BYTE)	/*   6250 byte */
#define	SEEK	(4000)		/* 1ms  (4MHz) */

/*---------------------------------------------------------------------*/

#define	REPEAT()	do{ if( !fdc_wait ) loop_flag = TRUE; }while(0)
#define	ICOUNT(x)	do{ if(  fdc_wait ) fdc.wait = (x);   }while(0)
#define	SCOUNT(d,x)	do{ if(  fdc_wait ) fdc.seek_wait[(d)] = (x);	\
			    else            fdc.seek_wait[(d)] = 1;   }while(0)

#define	ABS(x)		( ((x) >= 0)? (x) : -(x) )


int	fdc_ctrl( int interval )
{
  int	i, w;
  int loop_flag;

  if( ! fdc_wait ){			/* ウエイトなしの場合 */

    loop_flag = TRUE;				/* FDC処理を行なう*/

  }else{				/* ウエイトありの場合 */
    if( fdc.wait == -1 ){			/* wait==-1 なら、FDC処理 */
      loop_flag = TRUE;
    }else{					/* wait>0 なら、時間経過判定 */
      fdc.wait -= interval;
      if (fdc.wait<=0){ fdc.wait=0;   loop_flag = TRUE;  }
      else            {               loop_flag = FALSE; }
    }
  }

	/* seek_wait[] にドライブ別の、シーク時間が入っている		*/
	/*     > 1    … シーク中 (値は、完了までの4MHzステート数)	*/
	/*    1 → 0  … 1で、シーク完了。割り込みを発生して、0になる。	*/
	/*    0 → -1 … 割込発生中。SENSE INT で割込解除し、-1になる。	*/
	/*      -1    … 動作なし					*/

  for( i=0; i<MAX_DRIVE; i++ ){
    if( fdc.seek_wait[i] > 1 ){
      fdc.seek_wait[i] -= interval;
      if( fdc.seek_wait[i] <= 0 ) fdc.seek_wait[i] = 1;
    }
  }



  /******* FDC 処理。ウェイトなしなら、条件によっては、処理を繰り返す *******/

  while( loop_flag ){
    loop_flag = FALSE;


    switch( fdc.command ){

    case WAIT:		/* --------------------------- コマンド待ち ---	*/
      if( !( fdc.status & REQ_MASTER ) ){	/* コマンド受信した場合	*/
	switch( fdc.write & 0x1f ){
	case 0x06:	fdc.command = READ_DATA;		break;
	case 0x0c:	fdc.command = READ_DELETED_DATA;	break;
	case 0x0a:	fdc.command = READ_ID;			break;
	case 0x0d:	fdc.command = WRITE_ID;			break;
	case 0x05:	fdc.command = WRITE_DATA;		break;
	case 0x09:	fdc.command = WRITE_DELETED_DATA;	break;
	case 0x02:	fdc.command = READ_DIAGNOSTIC;		break;
	case 0x11:	fdc.command = SCAN_EQUAL;		break;
	case 0x19:	fdc.command = SCAN_LOW_OR_EQUAL;	break;
	case 0x1d:	fdc.command = SCAN_HIGH_OR_EQUAL;	break;
	case 0x0f:	fdc.command = SEEK;			break;
	case 0x07:	fdc.command = RECALIBRATE;		break;
	case 0x08:	fdc.command = SENSE_INT_STATUS;		break;
	case 0x04:	fdc.command = SENSE_DEVICE_STATUS;	break;
	case 0x03:	fdc.command = SPECIFY;			break;
	default:	fdc.command = INVALID;			break;
	}
	fdc.phase = C_PHASE;
	fdc.step  = 0;
	fdc.TC    = FALSE;
	loop_flag = TRUE;				/* 処理繰り返し */

      }else {					/* コマンドない場合	*/
	for( i=0; i<MAX_DRIVE; i++ ){			/* シーク完了を */
	  if( fdc.seek_wait[i] == 1 ){			/* チェックする */
	    if( (fdc.st0_seek[fdc.us] & ST0_NR) == 0){
	      fdc.pcn[ fdc.us ] = fdc.ncn[ fdc.us ];
	      if( !disk_not_exist( fdc.us ) ){
		disk_now_track( fdc.us, fdc.ncn[ fdc.us ]*2 + fdc.hd );
	      }
	    }
	    fdc_occur_interrupt();			/* 割り込み発生 */
	    fdc.seek_wait[i] = 0;
	    break;
	  }
	}
	ICOUNT( -1 );
      }
      break;




    case READ_DATA:		/* ---------------------- READ DATA ---	*/
    case READ_DELETED_DATA:	/* -------------- READ DELETED DATA ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.sk  = (fdc.write & 0x20) >> 5;
		    fdc.mf  = (fdc.write & 0x40) >> 6;
		    fdc.mt  = (fdc.write & 0x80) >> 7;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.c   =  fdc.write;		break;
	  case 3:   fdc.h   =  fdc.write;		break;
	  case 4:   fdc.r   =  fdc.write;		break;
	  case 5:   fdc.n   =  fdc.write;		break;
	  case 6:   fdc.eot =  fdc.write;		break;
	  case 7:   fdc.gpl =  fdc.write;		break;
	  case 8:   fdc.dtl =  fdc.write;		break;
	  }
	  if( fdc.step < 8 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("READ(%d) sk%d mf%d mt%d us%d hd%d eot%d\n",
		   fdc.command,fdc.sk,fdc.mf,fdc.mt,fdc.us,fdc.hd,fdc.eot);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  logfdc("C:%02X H:%02X R:%02X N:%02X  ",fdc.c,fdc.h,fdc.r,fdc.n);
	  fdc.skip_ddam_job = 0;
	  if( disk_read_data__id() ){	    /* ID検索開始 (ディスク挿入時真) */
	    if( fdc.st0 & ST0_IC ){		/* 検索失敗 */
	      logfdc("A\n");
	      fdc.step = 4;
	      ICOUNT( TRACK * 3/2 );
	    }
	    else				/* 検索成功、D/DAM不一致 */
	    if( ( fdc.command==READ_DATA         &&
		  ( fdc.st2 & ST2_CM )           )  ||
		( fdc.command==READ_DELETED_DATA &&
		  !( fdc.st2 & ST2_CM )          )  ){
	      if( fdc.sk ){			    /* スキップする時 */
		if( fdc_update_next_chrn()==0 ){	/* 次セクタを読む */
		  logfdc("B\n");
		  fdc.step = 0;
		  ICOUNT( ID + DATA );
		}else{					/* 次セクタは EOT */
		  fdc.st0 |= ST0_IC_AT;
		  fdc.st1 |= ST1_EN;
		  logfdc("C\n");
		  fdc.step = 4;
		  ICOUNT( ID );
		}
	      }else{				    /* スキップしない時 */
		fdc.skip_ddam_job = 1;
		disk_read_data__data();
		fdc.data_ptr = 0;
		if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
		else           fdc.counter = 128 << (fdc.n & 7);
		fdc.step ++;
		ICOUNT( ID );
	      }
	    }
	    else{				/* 検索成功、D/DAM一致 */
	      disk_read_data__data();
	      fdc.data_ptr = 0;
	      if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
	      else           fdc.counter = 128 << (fdc.n & 7);
	      fdc.step ++;
	      ICOUNT( ID );
	    }
	    REPEAT();
	  }else{			    /* ID検索不可 (ディスク未挿入) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( verbose_fdc )
	    if( fdc.TC && fdc.data_ptr==0 ) printf("FDC Read : no trans\n");
	  if( fdc.TC && fdc.data_ptr ){	    /* TC信号 有で、中断 */
	    if( fdc.st0 & ST0_IC ){		/* CRCエラー */
	      logfdc("D\n");
	    }else{				/* CRC正常 */
	      if( fdc.skip_ddam_job==0 ){	    /* D/DAM不一致スキップ無 */
		fdc_update_next_chrn();		    /* 以外は、次セクタを指す*/
		logfdc("E\n");
	      }else
		logfdc("F\n");
	    }
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* TC信号 無で、割込発生 */
	    fdc_occur_interrupt();
	    fdc.status = (fdc.status&0x0f)|FDC_BUSY|NON_DMA|DATA_IO|REQ_MASTER;
	    fdc.read   = data_buf[ fdc.data_ptr ++ ];
	    fdc.limit  = BYTE;
	    fdc.step ++;
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.limit -= interval;
	  if( fdc.limit<0 ){		    /* 一定時間経過でタイムアウト */
	    /* fdc.st1 |= ST1_OR; */
	    fdc.limit = 0;
	    if( verbose_fdc )
	      if( fdc_wait ) printf("FDC Read : Time Out\n");
	  }
	  if( fdc.TC ){			    /* TC信号 有で、中断 */
	    fdc_cancel_interrupt();
	    if( fdc.st0 & ST0_IC ){		/* CRCエラー */
	      logfdc("G\n");
	    }else{				/* CRC正常 */
	      if( fdc.skip_ddam_job==0 ){	    /* D/DAM不一致スキップ無 */
		fdc_update_next_chrn();		    /* 以外は、次セクタを指す*/
		logfdc("H\n");
	      }else
		logfdc("I\n");
	    }
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* データが読みだされた場合 */
	    if( !( fdc.status & REQ_MASTER ) ){
	      fdc_cancel_interrupt();
	      -- fdc.counter;
	      if( fdc.counter==0 ){fdc.step ++; ICOUNT(0);  fdc.limit += BYTE;}
	      else                {fdc.step --; ICOUNT( fdc.limit ); }
	    }
	    else{			    /* まだ、読まれない */
	      ICOUNT( -1 );
	    }
	  }
	  break;
	case 3:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( fdc.st0 & ST0_IC ){	    /* CRCエラーありの場合 */
	    logfdc("J\n");
	    fdc.step = 4;
	    ICOUNT( BYTE );
	    REPEAT();
	  }else{
	    if( fdc.TC ){		    /* TC信号 有で、中断 */
	      if( fdc.skip_ddam_job==0 ){	    /* D/DAM不一致スキップ無 */
		fdc_update_next_chrn();		    /* 以外は、次セクタを指す*/
		logfdc("K\n");
	      }else
		logfdc("L\n");
	      fdc.step = 4;
	      ICOUNT( 0 );
	      REPEAT();
	    }else{
	      fdc.limit -= interval;
	      if( fdc.limit <= 0 ){	    /* TC信号 無で、一定時間経過 */
		/*if( verbose_fdc ) printf("FDC Read : Multi\n");*/
		if( fdc.skip_ddam_job==0 ){	    /* D/DAM不一致スキップ無 */
		  if( fdc_update_next_chrn()==0 ){  /* 以外は、次セクタ処理  */
		    logfdc("M\n");
		    fdc.step = 0;
		  }else{			    /* 次セクタは EOT */
		    fdc.st0 |= ST0_IC_AT;
		    fdc.st1 |= ST1_EN;
		    logfdc("N\n");
		    fdc.step = 4;
		  }
		}else{				    /* D/DAM不一致スキップ無 */
		  fdc.st0 |= ST0_IC_AT;
		  fdc.st1 |= ST1_EN;
		  logfdc("O\n");
		  fdc.step = 4;
		}
		ICOUNT( 0 );
		REPEAT();
	      }else{			    /* まだTC信号ない */
		ICOUNT( fdc.limit );
	      }
	    }
	  }
	  break;
	case 4:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.TC = FALSE;	
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  ICOUNT( 0 );
	  REPEAT();
	  break;
	}
	break;


      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;		break;
	  case  8:   fdc.read = fdc.h;		break;
	  case 10:   fdc.read = fdc.r;		break;
	  case 12:   fdc.read = fdc.n;		break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* 読まれた */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* まだ読まれない */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;






    case READ_DIAGNOSTIC:	/* ---------------- READ DIAGNOSTIC ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.mf  = (fdc.write & 0x40) >> 6;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.c   =  fdc.write;		break;
	  case 3:   fdc.h   =  fdc.write;		break;
	  case 4:   fdc.r   =  fdc.write;		break;
	  case 5:   fdc.n   =  fdc.write;		break;
	  case 6:   fdc.eot =  fdc.write;		break;
	  case 7:   fdc.gpl =  fdc.write;		break;
	  case 8:   fdc.dtl =  fdc.write;		break;
	  }
	  if( fdc.step < 8 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.mt     = 0;
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("DIAG mf%d us%d hd%d eot%d\n",fdc.mf,fdc.us,fdc.hd,fdc.eot);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  logfdc("C:%02X H:%02X R:%02X N:%02X  ",fdc.c,fdc.h,fdc.r,fdc.n);
	  if( disk_read_diagnostic__id() ){ /* ID検索開始 (ディスク挿入時真) */
	    if( fdc.st0 & ST0_IC ){		/* 検索失敗 */
	      logfdc("A\n");
	      fdc.step = 4;
	      ICOUNT( TRACK + TRACK );
	    }
	    else{				/* 検索成功 */
	      disk_read_diagnostic__data();
	      fdc.data_ptr = 0;
	      if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
	      else           fdc.counter = 128 << (fdc.n & 7);
	      fdc.step ++;
	      ICOUNT( ID + DATA );
	    }
	    REPEAT();
	  }else{			    /* ID検索不可 (ディスク未挿入) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( verbose_fdc )
	    if( fdc.TC && fdc.data_ptr==0 ) printf("FDC Diag : no trans\n");
	  if( fdc.TC && fdc.data_ptr ){	    /* TC信号 有で、中断 */
	    if( fdc.st0 & ST0_IC ){		/* CRCエラー */
	      logfdc("B\n");
	    }else{				/* CRC正常 */
	      fdc_update_next_chrn();
	      logfdc("C\n");
	    }
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* TC信号 無で、割込発生 */
	    fdc_occur_interrupt();
	    fdc.status = (fdc.status&0x0f)|FDC_BUSY|NON_DMA|DATA_IO|REQ_MASTER;
	    fdc.read   = data_buf[ fdc.data_ptr ++ ];
	    fdc.limit  = BYTE;
	    fdc.step ++;
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.limit -= interval;
	  if( fdc.limit<0 ){		    /* 一定時間経過でタイムアウト */
	    /* fdc.st1 |= ST1_OR; */
	    fdc.limit = 0;
	    if( verbose_fdc )
	      if( fdc_wait ) printf("FDC Diag : Time Out\n");
	  }
	  if( fdc.TC ){			    /* TC信号 有で、中断 */
	    fdc_cancel_interrupt();
	    if( fdc.st0 & ST0_IC ){		/* CRCエラー */
	      logfdc("D\n");
	    }else{				/* CRC正常 */
	      fdc_update_next_chrn();
	      logfdc("E\n");
	    }
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* データが読みだされた場合 */
	    if( !( fdc.status & REQ_MASTER ) ){
	      fdc_cancel_interrupt();
	      -- fdc.counter;
	      if( fdc.counter==0 ){fdc.step ++; ICOUNT(0);  fdc.limit += BYTE;}
	      else                {fdc.step --; ICOUNT( fdc.limit ); }
	    }
	    else{			    /* まだ、読まれない */
	      ICOUNT( -1 );
	    }
	  }
	  break;
	case 3:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( fdc.TC ){			    /* TC信号 有で、中断 */
	    fdc_update_next_chrn();
	    logfdc("F\n");
	    fdc.step = 4;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{
	    fdc.limit -= interval;
	    if( fdc.limit <= 0 ){	    /* TC信号 無で、一定時間経過 */
	      /*if( verbose_fdc ) printf("FDC Diag : Multi\n");*/
	      if( fdc_update_next_chrn()==0 ){	    /* 次セクタ処理 */
		logfdc("G\n");
		fdc.step = 0;
	      }else{				    /* 次セクタはEOT */
		fdc.st0 |= ST0_IC_AT;
		fdc.st1 |= ST1_EN;
		logfdc("H\n");
		fdc.step = 4;
	      }
	      ICOUNT( 0 );
	      REPEAT();
	    }else{			    /* まだTC信号ない */
	      ICOUNT( fdc.limit );
	    }
	  }
	  break;
	case 4:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.TC = FALSE;	
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  ICOUNT( 0 );
	  REPEAT();
	  break;
	}
	break;


      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;		break;
	  case  8:   fdc.read = fdc.h;		break;
	  case 10:   fdc.read = fdc.r;		break;
	  case 12:   fdc.read = fdc.n;		break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* 読まれた */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* まだ読まれない */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;






    case READ_ID:		/* ------------------------ READ ID ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.sk  = (fdc.write & 0x20) >> 5;
		    fdc.mf  = (fdc.write & 0x40) >> 6;
		    fdc.mt  = (fdc.write & 0x80) >> 7;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  }
	  if( fdc.step < 1 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("READ ID mf%d us%d hd%d    ",fdc.mf,fdc.us,fdc.hd);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_read_id() ){		    /* ID検索開始 (ディスク挿入時真) */
	    fdc.step ++;
	    REPEAT();
	    ICOUNT( DATA + ID );
	  }else{			    /* ID検索不可 (ディスク未挿入) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.TC = FALSE;	
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  ICOUNT( 0 );
	  REPEAT();
	  break;
	}
	break;

      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;		break;
	  case  8:   fdc.read = fdc.h;		break;
	  case 10:   fdc.read = fdc.r;		break;
	  case 12:   fdc.read = fdc.n;		break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* 読まれた */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* まだ読まれない */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  logfdc("%02x %02x %02x %02x %02x %02x %02x\n",
		 fdc.st0,fdc.st1,fdc.st2,fdc.c,fdc.h,fdc.r,fdc.n);
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;






    case WRITE_ID:		/* ----------------------- WRITE ID ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.sk  = (fdc.write & 0x20) >> 5;
		    fdc.mf  = (fdc.write & 0x40) >> 6;
		    fdc.mt  = (fdc.write & 0x80) >> 7;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.n   =  fdc.write;		break;
	  case 3:   fdc.sc  =  fdc.write;		break;
	  case 4:   fdc.gpl =  fdc.write;		break;
	  case 5:   fdc.d   =  fdc.write;		break;
	  }
	  if( fdc.step < 5 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("W ID mf%d us%d n%d sc%d\n",fdc.mf,fdc.us,fdc.n,fdc.sc);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_id__check() ){	    /* 書込判定   (ディスク挿入時真) */
	    if( fdc.st0 & ST0_IC ){		/* 書き込み不能 */
	      logfdc("A\n");
	      fdc.step = 5;
	      ICOUNT( 0 );
	    }
	    else{				/* 書き込み可能 */
	      fdc.data_ptr = 0;
	      if( fdc.sc==0 ){
		fdc.counter = 256 * 4;
		if( verbose_fdc ) printf("FDC Wr ID : no sector\n");
	      }else
		fdc.counter = fdc.sc * 4;
	      fdc.step ++;
	      ICOUNT( TRACK/2 );
	    }
	    REPEAT();
	  }else{			    /* 書込判定不可 (ディスク未挿入) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( verbose_fdc )
	    if( fdc.TC && fdc.data_ptr==0 ) printf("FDC Wr ID : no trans\n");
	  if( fdc.TC && fdc.data_ptr ){	    /* TC信号 有で、中断 */
	    fdc.TC = FALSE;
	    fdc.step = 3;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			    /* TC信号 無で、割込発生 */
	    fdc_occur_interrupt();
	    fdc.status = (fdc.status&0x0f)|FDC_BUSY|NON_DMA|REQ_MASTER;
	    fdc.limit = BYTE;
	    fdc.step ++;
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.limit -= interval;
	  if( fdc.limit<0 ){		    /* 一定時間経過でタイムアウト */
	    /* fdc.st1 |= ST1_OR; */
	    fdc.limit = 0;
	    if( verbose_fdc )
	      if( fdc_wait ) printf("FDC Wr ID : Time Out\n");
	  }
	  if( fdc.TC ){			    /* TC信号 有で、中断 */
	    fdc_cancel_interrupt();
	    fdc.TC = FALSE;
	    fdc.step ++;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			    /* データが書き込まれた場合 */
	    if( !( fdc.status & REQ_MASTER ) ){
	      fdc_cancel_interrupt();
	      data_buf[ fdc.data_ptr ++ ] = fdc.write;
	      -- fdc.counter;
	      if( fdc.counter==0 ){fdc.step ++; ICOUNT(0);  }
	      else                {fdc.step --; ICOUNT( fdc.limit
					+ ((fdc.counter%4)!=0)? 0: ID+DATA ); }
	      logfdc("%02X ",fdc.write);
	      if((fdc.counter%4)==0) { logfdc("\n"); }
	    }
	    else{			    /* まだ、書かれない */
	      ICOUNT( -1 );
	    }
	  }
	  break;
	case 3:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( fdc.counter!=0 ){
	    if( verbose_fdc ) printf("FDC Write Id : missing\n");
	    while( (fdc.counter%4)!=0 ){		/* 4バイトに満たない */
	      data_buf[ fdc.data_ptr ++ ] = 0x00;	/* 部分は00Hで埋める */
	      -- fdc.counter;
	    }
	    fdc.sc = (4*fdc.sc - fdc.counter) /4;	/* その後 SC を更新 */
	  }
	  fdc.step ++;
	case 4:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_id__write() ){	    /* フォーマット(ディスク挿入時真)*/
	    fdc.step ++;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			   /*フォーマット不可(ディスク未挿入)*/
	    ICOUNT( 0 );
	  }
	  break;
	case 5:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  REPEAT();
	  ICOUNT( 0 );
	  break;
	}
	break;


      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;		break;
	  case  8:   fdc.read = fdc.h;		break;
	  case 10:   fdc.read = fdc.r;		break;
	  case 12:   fdc.read = fdc.n;		break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* 読まれた */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* まだ読まれない */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;




    case WRITE_DATA:		/* --------------------- WRITE DATA ---	*/
    case WRITE_DELETED_DATA:	/* ------------- WRITE DELETED DATA ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;
		    fdc.mf  = (fdc.write & 0x40) >> 6;
		    fdc.mt  = (fdc.write & 0x80) >> 7;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.c   =  fdc.write;		break;
	  case 3:   fdc.h   =  fdc.write;		break;
	  case 4:   fdc.r   =  fdc.write;		break;
	  case 5:   fdc.n   =  fdc.write;		break;
	  case 6:   fdc.eot =  fdc.write;		break;
	  case 7:   fdc.gpl =  fdc.write;		break;
	  case 8:   fdc.dtl =  fdc.write;		break;
	  }
	  if( fdc.step < 8 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.phase  = E_PHASE;
	    fdc.step   = 0;
	    logfdc("WRITE(%d) mf%d mt%d us%d eot%d\n",
		   fdc.command,fdc.mf,fdc.mt,fdc.us,fdc.eot);
	    ICOUNT( 0 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case E_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_data__check() ){   /* 書込判定   (ディスク挿入時真) */
	    if( fdc.st0 & ST0_IC ){		/* 書き込み不能 */
	      logfdc("A\n");
	      fdc.step = 7;
	    }else{				/* 書き込み可能 */
	      fdc.step ++;
	    }
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			    /* 書込判定不可 (ディスク未挿入) */
	    ICOUNT( 0 );
	  }
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_data__search() ){  /* ID検索開始 (ディスク挿入時真) */
	    if( fdc.st0 & ST0_IC ){		/* 検索失敗 */
	      logfdc("B\n");
	      fdc.step = 7;
	      ICOUNT( TRACK * 3/2 );
	    }else{				/* 検索成功 */
	      logfdc("C:%02X H:%02X R:%02X N:%02X  ",fdc.c,fdc.h,fdc.r,fdc.n);
	      fdc.data_ptr = 0;
	      if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
	      else           fdc.counter = 128 << (fdc.n & 7);
	      fdc.step ++;
	      ICOUNT( ID );
	    }
	    REPEAT();
	  }else{			    /* ID検索不可 (ディスク未挿入) */
	    ICOUNT( 0 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( verbose_fdc )
	    if( fdc.TC && fdc.data_ptr==0 ) printf("FDC Write : no trans\n");
	  if( fdc.TC && fdc.data_ptr ){	    /* TC信号 有で、中断 */
	    logfdc("D\n");
	    fdc.step = 4;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* TC信号 無で、割込発生 */
	    fdc_occur_interrupt();
	    fdc.status = (fdc.status&0x0f)|FDC_BUSY|NON_DMA|REQ_MASTER;
	    fdc.limit = BYTE;
	    fdc.step ++;
	    ICOUNT( -1 );
	  }
	  break;
	case 3:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.limit -= interval;
	  if( fdc.limit<0 ){		    /* 一定時間経過でタイムアウト */
	    /* fdc.st1 |= ST1_OR; */
	    fdc.limit = 0;
	    if( verbose_fdc )
	      if( fdc_wait ) printf("FDC Write : Time Out\n");
	  }
	  if( fdc.TC ){			    /* TC信号 有で、中断 */
	    fdc_cancel_interrupt();
	    logfdc("E\n");
	    fdc.step ++;
	    ICOUNT( fdc.counter * BYTE );
	    REPEAT();
	  }else{			    /* データが書き込まれた場合 */
	    if( !( fdc.status & REQ_MASTER ) ){
	      fdc_cancel_interrupt();
	      data_buf[ fdc.data_ptr ++ ] = fdc.write;
	      -- fdc.counter;
	      if( fdc.counter==0 ){fdc.step ++; ICOUNT(0);  fdc.limit += BYTE;}
	      else                {fdc.step --; ICOUNT( fdc.limit ); }
	    }
	    else{			    /* まだ、書かれない */
	      ICOUNT( -1 );
	    }
	  }
	  break;
	case 4:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  while( fdc.counter ){
	    data_buf[ fdc.data_ptr ++ ] = 0x00;		/*	セクタサイズ */
	    -- fdc.counter;				/*	に満たない時 */
	  }						/*	は、00Hで    */
	  if( fdc.n==0 && fdc.dtl<128 ){		/*	埋める。     */
	    for( i=0; i<128-fdc.dtl; i++ ){
	      data_buf[ fdc.data_ptr ++ ] = 0x00;
	    }
	  }
	  fdc.step ++;
	case 5:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( disk_write_data__write() ){   /* 書き込み (ディスク挿入時真)   */
	    fdc.step ++;
	    ICOUNT( 0 );
	    REPEAT();
	  }else{			    /* 書き込み失敗 (ディスク未挿入) */
	    ICOUNT( 0 );
	  }
	  break;
	case 6:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( fdc.st0 & ST0_IC ){	    /* 異常ありで、即終了 */
	    logfdc("F\n");
	    fdc.step ++;
	    ICOUNT( BYTE );
	    REPEAT();
	  }else{			    /* 異常なしで、TCを待つ */
	    if( fdc.TC ){			/* TC信号 有で、中断 */
	      fdc_update_next_chrn();
	      logfdc("G\n");
	      fdc.step ++;
	      ICOUNT( 0 );
	      REPEAT();
	    }else{
	      fdc.limit -= interval;
	      if( fdc.limit <= 0 ){		/* TC信号 無で、一定時間経過 */
		/*if( verbose_fdc ) printf("FDC Write : Multi\n");*/
		if( fdc_update_next_chrn()==0 ){	    /* 次セクタ処理 */
		  logfdc("H\n");
		  fdc.step = 1;
		}else{					    /* 次セクタはEOT */
		  fdc.st0 |= ST0_IC_AT;
		  fdc.st1 |= ST1_EN;
		  logfdc("I\n");
		  fdc.step = 7;
		}
		ICOUNT( 0 );
		REPEAT();
	      }else{				/* まだTC信号ない */
		ICOUNT( fdc.limit );
	      }
	    }
	  }
	  break;
	case 7:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.TC = FALSE;
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	  fdc.phase  = R_PHASE;
	  fdc.step   = 0;
	  ICOUNT( 0 );
	  REPEAT();
	  break;
	}
	break;
	

      case R_PHASE:
	switch( fdc.step ){
	case 0:      case 2:      case 4:      case 6:
	case 8:      case 10:     case 12:
	  if( fdc.step==0 ) fdc_occur_interrupt();
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  switch( fdc.step ){
	  case  0:   fdc.read = fdc.st0;	break;
	  case  2:   fdc.read = fdc.st1;	break;
	  case  4:   fdc.read = fdc.st2;	break;
	  case  6:   fdc.read = fdc.c;	break;
	  case  8:   fdc.read = fdc.h;	break;
	  case 10:   fdc.read = fdc.r;	break;
	  case 12:   fdc.read = fdc.n;	break;
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 1:      case 3:      case 5:      case 7:
	case 9:      case 11:	  case 13:
	  if( !( fdc.status & REQ_MASTER ) ){		/* 読まれた */
	    if( fdc.step==1 ) fdc_cancel_interrupt();
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==14 ) REPEAT();
	  }else{					/* まだ読まれない */
	    ICOUNT( -1 );
	  }
	  break;
	case 14:
	  fdc.read = 0xff;
	  fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;




    case SEEK:		/* ----------------------------------- SEEK ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  case 2:   fdc.ncn[ fdc.us ] =  fdc.write;	break;
	  }
	  if( fdc.step < 2 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    if( fdc.us >= NR_DRIVE ||			/* 無効ドライブ or */
		fdc.status & (1<<fdc.us) ){		/* 現在シーク中    */

	      fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	      fdc.st0_seek[fdc.us] = ST0_IC_AT | ST0_SE | ST0_NR | fdc.us;
	      SCOUNT( fdc.us, 1 );

	    }else{
	      fdc.status = (fdc.status&0x0f) | REQ_MASTER | (1<<fdc.us);
	      fdc.st0_seek[fdc.us] = ST0_IC_NT | ST0_SE | fdc.us;
	      SCOUNT( fdc.us,
		      1+ABS(fdc.ncn[fdc.us]-fdc.pcn[fdc.us])*fdc.srt_ms*SEEK);
	    }
	    logfdc("SEEK %d-%x (Tr.%d)\n", 
		   fdc.us, fdc.ncn[fdc.us], fdc.ncn[fdc.us]*2 );
	    fdc.command = WAIT;
	    fdc.step = 0;
	    ICOUNT( -1 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;
      }
      break;


    case RECALIBRATE:	/* ---------------------------- RECALIBRATE ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  }
	  if( fdc.step < 1 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.ncn[ fdc.us ] = 0;
	    if( fdc.us >= NR_DRIVE ||			/* 無効ドライブ or */
		fdc.status & (1<<fdc.us) ){		/* 現在シーク中    */

	      fdc.status = (fdc.status&0x0f) | REQ_MASTER;
	      fdc.st0_seek[fdc.us] = ST0_IC_AT | ST0_SE | ST0_NR | fdc.us;
	      SCOUNT( fdc.us, 1 );

	    }else{
	      fdc.status = (fdc.status&0x0f) | REQ_MASTER | (1<<fdc.us);
	      fdc.st0_seek[fdc.us] = ST0_IC_NT | ST0_SE | fdc.us;
	      SCOUNT( fdc.us,
		      1+ABS(fdc.ncn[fdc.us]-fdc.pcn[fdc.us])*fdc.srt_ms*SEEK);
	    }
	    logfdc("RECALIBRATE %d\n",fdc.us);
	    fdc.command = WAIT;
	    fdc.step = 0;
	    ICOUNT( -1 );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;
      }
      break;




    case SENSE_INT_STATUS:	/* --------------- SENSE_INT_STATUS ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  }
	  {
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = R_PHASE;
	    fdc.step   = 0;
	    logfdc("SENSE INT\n");
	    ICOUNT( RD );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;


      case R_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  for( i=0; i<MAX_DRIVE; i++ )	/* シーク完了(割込発生)ドライブ検索 */
	    if( fdc.seek_wait[i]==0 ) break;
	  fdc.sense_us = i;
	  fdc.step ++;
	case 1:	case 3:	/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  if( fdc.sense_us==MAX_DRIVE ){	/* シーク完了ドライブなし */
	    switch( fdc.step ){
	    case  1:   fdc.read = ST0_IC_IC;	break;
	    case  3:   fdc.read = 0;		break;
	    }
	  }else{				/* シーク完了ドライブあり */
	    fdc_cancel_interrupt();
	    fdc.seek_wait[ fdc.sense_us ] = -1;
	    switch( fdc.step ){
	    case  1:   fdc.read = fdc.st0_seek[ fdc.sense_us ];	break;
	    case  3:   fdc.read = fdc.pcn[ fdc.sense_us ];	break;
	    }
	  }
	  fdc.step ++;
	  ICOUNT( -1 );
	  break;
	case 2:	case 4:	/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( !( fdc.status & REQ_MASTER ) ){		/* 読まれた */
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    if( fdc.step==5 ) REPEAT();
	  }else{					/* まだ読まれない */
	    ICOUNT( -1 );
	  }
	  break;
	case 5:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.read = 0xff;
	  if( fdc.sense_us < MAX_DRIVE )
	    fdc.status &= ~(1<<fdc.sense_us);
	  fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;



    case SENSE_DEVICE_STATUS:	/* ------------ SENSE_DEVICE_STATUS ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  case 1:   fdc.us  =  fdc.write & 0x03;
		    fdc.hd  = (fdc.write & 0x04) >> 2;	break;
	  }
	  if( fdc.step < 1 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = R_PHASE;
	    fdc.step   = 0;
	    logfdc("SENSE DEV %d\n",fdc.us);
	    ICOUNT( RD );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;

      case R_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  if( fdc.us >= NR_DRIVE ){

	    fdc.read = ST3_FT | ((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );

	  }else{
	    if( disk_not_exist( fdc.us ) ||
		(disk_ex_drv & (1 << fdc.us)) ){ /* ディスク入れ替えチェック */
							    /* thanks peach! */
	      disk_ex_drv ^= (1 << fdc.us); /* ドライブ番号のビット反転 */
	      fdc.read =((fdc.status & (1<<fdc.us)) ? 0 : ST3_RY ) |
			((fdc.pcn[fdc.us]==0) ? ST3_T0 : 0 ) |
			((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );

	    }else{

	      fdc.read =((fdc.status & (1<<fdc.us)) ? 0 : ST3_RY ) |
	      		((drive[fdc.us].protect==DISK_PROTECT_TRUE)?ST3_WP:0) |
			((fdc.pcn[fdc.us]==0) ? ST3_T0 : 0 ) | ST3_TS |
			((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );

	    }
	  }
	  fdc.step ++;
	  ICOUNT( WR );
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( !( fdc.status & REQ_MASTER ) ){		/* 読まれた */
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    REPEAT();
	  }else{					/* まだ読まれない */
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.read = 0xff;
	  fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;



    case SPECIFY:	/* -------------------------------- SPECIFY ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd    = fdc.write & 0x1f;		break;
	  case 1:   fdc.srt_ms = 32 - ((fdc.write>>4)&0x0f)*2;
		    fdc.hut    = fdc.write & 0x0f;		break;
	  case 2:   fdc.hlt_nd = fdc.write;			break;
	  }
	  if( fdc.step < 2 ){
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
	    fdc.step ++;
	    ICOUNT( RD );
	  }else{
	    fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	    fdc.command = WAIT;
	    fdc.step = 0;
	    logfdc("SPECIFY\n");
	    ICOUNT( -1 );
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;
      }
      break;



    case INVALID:	/* -------------------------------- Invalid ---	*/
      switch( fdc.phase ){
      case C_PHASE:
	if( !( fdc.status & REQ_MASTER ) ){
	  switch( fdc.step ){
	  case 0:   fdc.cmd =  fdc.write & 0x1f;	break;
	  }
	  {
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	    fdc.phase  = R_PHASE;
	    fdc.step   = 0;
	    ICOUNT( RD );
	    REPEAT();
	  }
	}else{
	  ICOUNT( -1 );
	}
	break;

      case R_PHASE:
	switch( fdc.step ){
	case 0:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
	  {
	    fdc.read = ST0_IC_IC;
	  }
	  fdc.step ++;
	  ICOUNT( WR );
	  break;
	case 1:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  if( !( fdc.status & REQ_MASTER ) ){		/* 読まれた */
	    fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	    fdc.step ++;
	    ICOUNT( WR );
	    REPEAT();
	  }else{					/* まだ読まれない */
	    ICOUNT( -1 );
	  }
	  break;
	case 2:		/* - - - - - - - - - - - - - - - - - - - - - - - - - */
	  fdc.read = 0xff;
	  fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	  fdc.command = WAIT;
	  fdc.step = 0;
	  ICOUNT( -1 );
	  break;
	}
	break;
      }
      break;


    default:		/* --------------------------------------------	*/
      printf("FDC COM=%d\n",fdc.command );
      fflush(0);
      emu_mode = MONITOR;
      break;

    }
  }




  if( ! fdc_wait ){			/* ウエイトなしの場合 */
    						/* ポートアクセスが起こる迄*/
    w = 999999;					/* 永久に待つ。		   */

  }else{				/* ウエイトありの場合 */
    if( fdc.wait==-1 ) w = 9999999;		/* wait==-1なら、永久に待つ  */
    else               w = fdc.wait;		/* wait>=0なら、一定時間待つ */
  }

  if( fdc.command == WAIT ){		/* 待機中のみ、シーク完了時間を確認 */
    for( i=0; i<MAX_DRIVE; i++ )
      if( fdc.seek_wait[i] && fdc.seek_wait[i] < w ) w = fdc.seek_wait[i];
  }
  return w;					
}




/************************************************************************/
/* ドライブの状態を返す関数						*/
/************************************************************************/
int	get_drive_ready( int drv )
{
  if( fdc.command==WAIT ) return 1;
  else{
    if( fdc.us==drv ) return 0;
    else              return 1;
  }
}







/************************************************************************/
/* セクタ間を埋める                                                     */
/************************************************************************/
/*
 * READ DIAG のセクタ間のデータを埋める処理は、peach氏により提供されました。
 */
#define IF_WITHIN_DATA_BUF_SIZE(s)	\
if (s >= DATA_BUF_SIZE) {		\
    printf("Buffer over flow\n");	\
    fflush(stdout);			\
    return(-1);				\
} else

#define RET_GAP_ERR(cond) if (cond != TRUE) return(-1);

/*
 * GAP3 の計算
 */
INLINE
int calc_gap3_size(int n, Uchar fdc_mf)
{
    int gap3_size;

    /* GAP3 のサイズは決まっているわけではないのであくまで標準的な値 */
    switch (n) {
    case 1:	gap3_size = 27;	break;
    case 2:	gap3_size = 42;	break;
    case 3:	gap3_size = 58;	break;
    default:	gap3_size = 58; /* その他は分かんね */
    }
    if (fdc_mf) gap3_size *= 2;	/* 倍密度 */
    return(gap3_size);
}

INLINE
int input_safely_data(int ptr, int *ad, int data, int size)
{
    if (size == 0) return(TRUE);

    if (ptr + *ad + size < DATA_BUF_SIZE) {
	memset(&data_buf[ptr + *ad], data, size);
	*ad += size;
	return(TRUE);
    } else {
	printf("Buffer overflow\n");
	fflush(stdout);
	return(FALSE);
    }
}

static int fill_sector_gap(int ptr, int drv, Uchar fdc_mf)
{
    int   sync_size, am_size;
    int   gap0_size, gap1_size, gap2_size, gap3_size, gap4_size;
    int   track_size;
    Uchar gap;
    Uchar undel;
    int   size;
    int   tmp_size;
    
    if (fdc_mf) {
	/* 倍密度 */
	track_size = 6250;
	gap0_size = 80;
	sync_size = 12;
	am_size   = 3 + 1;
	gap1_size = 50;
	gap2_size = 22;
	gap = 0x4e;
	/*if      (sec_buf.sec_nr <= 8)  gap4_size = 654;
	  else if (sec_buf.sec_nr <= 15) gap4_size = 400;
	  else                           gap4_size = 598;*/
    } else {
	/* 単密度 */
	track_size = 3100;
	gap0_size = 40;
	sync_size = 6;
	am_size = 1;
	gap1_size = 26;
	gap2_size = 11;
	gap = 0xff;
	/*if      (sec_buf.sec_nr <= 8)  gap4_size = 311;
	  else if (sec_buf.sec_nr <= 15) gap4_size = 170;
	  else                           gap4_size = 247;*/
    }
    gap3_size = calc_gap3_size(sec_buf.n, fdc_mf);

    size = 0;
    /* DATA フィールドの最後尾 */
    size += 2;			/* DATA CRC */

    /* GAP3 */
    RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap3_size));

    /* 最終セクタならプリアンブル + ポストアンブル */
    if (drive[drv].sec + 1 >= drive[drv].sec_nr) {
	/* ポストアンブル (GAP4) のサイズを調べる */
	tmp_size = gap0_size + sync_size + am_size + gap1_size;
	/* 全てのセクタで GAP をカウント */
	do {
	    disk_next_sec(drv);
	    tmp_size += sync_size + am_size + 4 + 2; /* ID フィールド */
	    tmp_size += gap2_size;
	    tmp_size += sync_size + am_size + sec_buf.size + 2;
	    tmp_size += calc_gap3_size(sec_buf.n, fdc_mf);
	} while (drive[drv].sec + 1 < drive[drv].sec_nr);
	gap4_size = track_size - tmp_size;
	if (gap4_size < 0) {
	    printf("Abnormal sector\n");
	    return(-1);
	}

	
	RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap4_size));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0xc2, am_size - 1));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0xfc, 1));
	RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap1_size));
    }
  
    /* 次のセクタ */
    disk_next_sec(drv);

    /* ID フィールド */
    RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xa1, am_size - 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xfe, 1));
  
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.c, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.h, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.r, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.n, 1));
    size += 2;			/* ID CRC */

    /* GAP2 */
    RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap2_size));

    /* DATA フィールド */
    RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xa1, am_size - 1));

    if (sec_buf.deleted == DISK_DELETED_TRUE) undel = 0xf8;
    else undel = 0xfb;
    RET_GAP_ERR(input_safely_data(ptr, &size, undel, 1));
  
    return(size);
}




/****************************************************************/	
/* サスペンド／レジューム					*/
/****************************************************************/	
static	char	filename[2][1024];

static	T_SUSPEND_W	suspend_fdc_work[]=
{
  { TYPE_STR,	&filename[0][0],		},
  { TYPE_STR,	&filename[1][0],		},
  { TYPE_CHAR,	&drive[0].selected_image,	},
  { TYPE_CHAR,	&drive[1].selected_image,	},

  { TYPE_INT,	&FDC_flag,			},
  { TYPE_INT,	&fdc_wait,			},

  { TYPE_INT,	&fdc.command,	},
  { TYPE_INT,	&fdc.phase,	},
  { TYPE_INT,	&fdc.step,	},
  { TYPE_INT,	&fdc.counter,	},
  { TYPE_INT,	&fdc.data_ptr,	},

  { TYPE_INT,	&fdc.limit,	},
  { TYPE_INT,	&fdc.wait,	},
  { TYPE_INT,	&fdc.seek_wait[0],	},
  { TYPE_INT,	&fdc.seek_wait[1],	},
  { TYPE_INT,	&fdc.seek_wait[2],	},
  { TYPE_INT,	&fdc.seek_wait[3],	},

  { TYPE_BYTE,	&fdc.status,	},
  { TYPE_BYTE,	&fdc.read,	},
  { TYPE_BYTE,	&fdc.write,	},
  { TYPE_BYTE,	&fdc.TC,	},

  { TYPE_CHAR,	&fdc.cmd,	},
  { TYPE_CHAR,	&fdc.sk,	},
  { TYPE_CHAR,	&fdc.mf,	},
  { TYPE_CHAR,	&fdc.mt,	},
  { TYPE_CHAR,	&fdc.us,	},
  { TYPE_CHAR,	&fdc.hd,	},
  { TYPE_CHAR,	&fdc.c,		},
  { TYPE_CHAR,	&fdc.h,		},
  { TYPE_CHAR,	&fdc.r,		},
  { TYPE_CHAR,	&fdc.n,		},
  { TYPE_CHAR,	&fdc.eot,	},
  { TYPE_CHAR,	&fdc.gpl,	},
  { TYPE_CHAR,	&fdc.dtl,	},
  { TYPE_CHAR,	&fdc.d,		},
  { TYPE_CHAR,	&fdc.sc,	},
  { TYPE_CHAR,	&fdc.stp,	},
  { TYPE_CHAR,	&fdc.srt_ms,	},
  { TYPE_CHAR,	&fdc.hut,	},
  { TYPE_CHAR,	&fdc.hlt_nd,	},
  { TYPE_CHAR,	&fdc.ncn[0],	},
  { TYPE_CHAR,	&fdc.ncn[1],	},
  { TYPE_CHAR,	&fdc.ncn[2],	},
  { TYPE_CHAR,	&fdc.ncn[3],	},
  { TYPE_CHAR,	&fdc.pcn[0],	},
  { TYPE_CHAR,	&fdc.pcn[1],	},
  { TYPE_CHAR,	&fdc.pcn[2],	},
  { TYPE_CHAR,	&fdc.pcn[3],	},
  { TYPE_CHAR,	&fdc.st0,	},
  { TYPE_CHAR,	&fdc.st1,	},
  { TYPE_CHAR,	&fdc.st2,	},
  { TYPE_CHAR,	&fdc.st3,	},
  { TYPE_CHAR,	&fdc.st0_seek[0],	},
  { TYPE_CHAR,	&fdc.st0_seek[1],	},
  { TYPE_CHAR,	&fdc.st0_seek[2],	},
  { TYPE_CHAR,	&fdc.st0_seek[3],	},
  { TYPE_CHAR,	&fdc.sense_us,		},
  { TYPE_CHAR,	&fdc.skip_ddam_job,	},
};


int	suspend_fdc( FILE *fp, long offset, long offset2 )
{
  int	ret;

  if( drive[0].filename ) strncpy( filename[0], drive[0].filename, 1024 );
  else                             filename[0][0] = '\0';
  if( drive[1].filename ) strncpy( filename[1], drive[1].filename, 1024 );
  else                             filename[1][0] = '\0';

  ret = suspend_work( fp, offset, 
		      suspend_fdc_work, 
		      countof(suspend_fdc_work) );

  if( ret<0 ) return FALSE;

  if( verbose_suspend )
    printf( "suspend <%s>: %d byte\n", __FILE__, ret );
  if( (int)sizeof( ((suspend_0*)0)->fdc ) < ret ){
    fprintf( stderr, "SUSPEND ERROR : Size Overflow in <%s>\n",__FILE__);
    main_exit(4);
  }

  if( ex_fseek( fp, offset2, SEEK_SET ) == 0 ){
    if(ex_fwrite( data_buf, sizeof(Uchar), 0x4000, fp ) != 0x4000 ) return FALSE;
  }else{
    return FALSE;
  }

  return TRUE;
}


int	resume_fdc(  FILE *fp, long offset, long offset2 )
{
  int	ret = resume_work( fp, offset, 
			   suspend_fdc_work, 
			   countof(suspend_fdc_work) );

  if( ret<0 ) return FALSE;

  if( ex_fseek( fp, offset2, SEEK_SET ) == 0 ){
    if(ex_fread( data_buf, sizeof(Uchar), 0x4000, fp ) != 0x4000 ) return FALSE;
  }else{
    return FALSE;
  }
  return TRUE;
}



void	fdc_init_at_resume( void )
{
  int drv;

  for( drv=0; drv<2; drv ++ ){

    if( filename[ drv ][0] != '\0' ){

      if( verbose_proc )
	printf( "DRIVE %d: [ %s ] image no.( %d ) ... ", 
		drv+1, filename[drv], drive[drv].selected_image+1 );

      if( disk_insert( drv, filename[drv], drive[drv].selected_image ) == 0 ){
	if( verbose_proc )
	  printf( "OK\n" );
      }
    }
  }
}


















/* デバッグ用の関数 */
void monitor_fdc(void)
{
  printf("com = %d phs = %d  step = %d\n",fdc.command,fdc.phase,fdc.step);
  printf("FDC flag = %d\n",FDC_flag);
#if 0
  printf("\n");
  printf("fp       = %d , %d\n", drive[0].fp,       drive[1].fp      );
  printf("track    = %d , %d\n", drive[0].track,    drive[1].track   );
  printf("sec_nr   = %d , %d\n", drive[0].sec_nr,   drive[1].sec_nr  );
  printf("sec      = %d , %d\n", drive[0].sec,      drive[1].sec       );
  printf("sec_pos  = %d , %d\n", drive[0].sec_pos,  drive[1].sec_pos   );
  printf("track_top= %d , %d\n", drive[0].track_top,drive[1].track_top );
  printf("disk_top = %d , %d\n", drive[0].disk_top, drive[1].sec     );
  printf("type     = %d , %d\n", drive[0].type,     drive[1].type );
  printf("protect  = %d , %d\n", drive[0].protect,  drive[1].protect     );

  printf("\n");
  {
    int	i,j,k;
    for(k=0;k<(128<<(fdc.n&7))/256;k++){
      for(i=0;i<16;i++){
	for(j=0;j<16;j++){
	  printf("%02X ",data_buf[k*256+i*16+j]);
	}
	printf("\n");
      }
    }
  }
  printf("\n");
#endif
}
