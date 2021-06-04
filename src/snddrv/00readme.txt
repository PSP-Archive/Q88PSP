
※ このディレクトリ以下には、MAME-0.71/XMAME-0.71.1 のソースが含まれています。
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  QUASI88 のコンパイル時にサウンド出力を有効にしておくと、このディレクトリ
以下のファイルが組み込まれ、MAME/XMAME のサウンド出力機能 (サウンドドライバ)
が使用することができるようになります。

なお、この場合コンパイルしたバイナリは、MAME および XMAME のライセンスが適用
されますのでご注意ください。このディレクトリ以下のソースファイルの再配布等に
ついても同様です。


MAME のライセンスについては、このディレクトリにある

	mame.txt

を参照してください。(XMAMEのライセンスもこれに準拠します)


(*) MAME は、Nicola Salmoria 氏 と The MAME Team が著作権を有する、マルチ
    アーケードマシンエミュレータです。
    XMAME は、xmame project による MAME の UNIX移植版です。

    MAME、XMAME についての、より多くの情報は以下のページを御覧下さい。

	MAME  Home Page ==>> http://www.mame.net/
	XMAME Home Page ==>> http://x.mame.net/


-------------------------------------------------------------------------------
xmame-0.71.1 のサウンドドライバを組み込みにあたり、以下のファイルを
使用しました。これらのうちいくつかは、QUASI88 向けに改変されています。

もともとのファイルのあった場所は以下の通りです。

xmame-0.71.1/src/sndintrf.c
xmame-0.71.1/src/sndintrf.h
xmame-0.71.1/src/sound/2203intf.c
xmame-0.71.1/src/sound/2203intf.h
xmame-0.71.1/src/sound/2608intf.c
xmame-0.71.1/src/sound/2608intf.h
xmame-0.71.1/src/sound/ay8910.c
xmame-0.71.1/src/sound/ay8910.h
xmame-0.71.1/src/sound/filter.c
xmame-0.71.1/src/sound/filter.h
xmame-0.71.1/src/sound/fm.c
xmame-0.71.1/src/sound/fm.h
xmame-0.71.1/src/sound/mixer.c
xmame-0.71.1/src/sound/mixer.h
xmame-0.71.1/src/sound/streams.c
xmame-0.71.1/src/sound/streams.h
xmame-0.71.1/src/sound/ymdeltat.c
xmame-0.71.1/src/sound/ymdeltat.h
xmame-0.71.1/src/unix/sysdep/dsp-drivers/sdl.c



xmame のソースで必要としているヘッダファイルから、
必要な項目を抜き出してまとめたのが、以下のファイルです。

	snddrv/xmame-quasi88.h


以下は、コンパイル用のダミーのヘッダファイルです。

	snddrv/xmame.h
	snddrv/devices.h
	snddrv/driver.h
	snddrv/osd_cpu.h
	snddrv/state.h

-------------------------------------------------------------------------------
MAME/XMAME と QUASI88 とのインターフェイス部として、以下のファイルを
新規に追加しました。

	snddrv/xmame-quasi88.c
	snddrv/xmame-quasi88.h

-------------------------------------------------------------------------------
MAME/XMAME のサウンドドライバで BEEP 音を出力させるために、以下のファイルを
新規に追加しました。

	snddrv/beepintf.c
	snddrv/beep.c
	snddrv/beep.h
