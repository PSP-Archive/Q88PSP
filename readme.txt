Q88P Version 0.7
　QUASI88をPSPに移植したものです。

■使用方法
  PC-8801のBIOSイメージをEBOOT.PBPと同一ディレクトリに置いて下さい。
　ディスクイメージの.d88ファイルは、EBOOT.PBP以下のフォルダに置いて下さい。

■ライセンス
　バイナリおよびソースコードのライセンスは基本的にQUASI88のライセンスに従いますが、
　syscall.c、pg.cなど影響によりGPLとして扱うことも可能です。

■PSP移植版開発者
　某吉
  武藤

■履歴

　Version 0.7 080128
　* フルスクリーンに対応。
　　->ソフトウェアキーボードは変更なし。フルスクリーンでも問答無用で画面上にキーボードを描画する。

　Version 0.6 061229
　* 音欠けを修正。
　　-> 修正はしたが、そのためにノイズがのるようになった。後画面更新などの処理が重いところで音飛びするようになった。
　* ステートセーブ/ロード機能実装。
　　-> まれにLoad後にハングアップするので、過信は禁物。
　* 色調補完時の色化けを修正。
　* かな/CAPSキーのロック状態でソフトウェアキーボードのキートップ表示を変更。
　* ソフトウェアキーボード上からCAPSキーとかなキーをロックできるようにした。

　Version 0.5 061031
　* ソフトウェアキーボード実装
　　-> ソフトウェアキーボード切り替えにRトリガーを使用しているので、Rトリガーのキー設定不可に。
　* Start、Selectも使えるように
　　-> キーセッティングに修正が入ってしまったために過去のconfigfileと互換性がなくなってしまった。今回のVersionで作り直すことになる。
　* パレット設定のバグを修正
　* F6〜F10の入力を修正
　* CAPSキーとかなキーのロック方法を修正。よりやりやすくなった

　Version 0.4 061019
　* ディスクへのセーブに対応
　* SubCPUの動作タイミングを追加
　* パレットの設定を追加
　* ファイラーの表示色を少し変更
　* キーコンフィグを修正、Delete、Insert、Backspaceが効くように。
　　CAPSとかなキーのロックにも対応したがロックと解除がしにくい。
　* 諸々のバグ取り
　　ファイラーのバグ、コンフィグファイル保存時のバグ、ファイルオープンの方法を修正

　Version 0.3 061001
　* コンフィグ設定をファイルに残す処理を追加。起動時に自動で読み込みます。
　* ファイラーを追加
　* キーコンフィグを修正
　* BASIC MODEを追加

  Version 0.2 060922 
　* このドキュメントを追加
  * メニュー画面を追加

　Version 0.1 06090X
  * 最初のリリース




