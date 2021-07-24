
        ＝ TOPPERS/JSPカーネル ユーザズマニュアル ＝
      （monitor、netDeviceの説明、newlib標準入出力の対応）

        （Release 1.4.3 対応，最終更新: 11-27-2010）

------------------------------------------------------------------------
 TOPPERS/JSP Kernel
     Toyohashi Open Platform for Embedded Real-Time Systems/
     Just Standard Profile Kernel

 Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
                             Toyohashi Univ. of Technology, JAPAN
 Copyright (C) 2003-2004 by Takagi Nobuhisa
 Copyright (C) 2003-2010 by Ryosuke Takeuchi
                     GJ Business Division RICOH COMPANY,LTD. JAPAN

 上記著作権者は，以下の (1)〜(4) の条件か，Free Software Foundation 
 によって公表されている GNU General Public License の Version 2 に記
 述されている条件を満たす場合に限り，本ソフトウェア（本ソフトウェア
 を改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 利用と呼ぶ）することを無償で許諾する．
 (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
     権表示，この利用条件および下記の無保証規定が，そのままの形でソー
     スコード中に含まれていること．
 (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
     用できる形で再配布する場合には，再配布に伴うドキュメント（利用
     者マニュアルなど）に，上記の著作権表示，この利用条件および下記
     の無保証規定を掲載すること．
 (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
     用できない形で再配布する場合には，次のいずれかの条件を満たすこ
     と．
   (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
       作権表示，この利用条件および下記の無保証規定を掲載すること．
   (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
       報告すること．
 (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
     害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．

 本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 よびTOPPERSプロジェクトは，本ソフトウェアに関して，その適用可能性も
 含めて，いかなる保証も行わない．また，本ソフトウェアの利用により直
 接的または間接的に生じたいかなる損害に関しても，その責任を負わない．
------------------------------------------------------------------------
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

１．monitorについて

１−１．解説
本モニタは、TOPPERSプロジェクト教育ＷＧの実装教育用に作成されたプログラム
です。TOPPERS/JSP1.4.1上でタスクとして実行します。実行確認は以下の４つの
環境環境で実行確認を行っています。
１）m16c(OAKS16-miniとOAKS16)   ルネサステクノロジ製TM
２）h8(AKIH8-3048,AKIH8-3069F)  PizzaFactory2+GNU(h8300-hms)
                                cygwin+GNU(h8300-hms)
３）sh2(hsb7616it)              cygwin+GNU(sh-hitachi-elf)

１−２．コンソールの指定
タスクモニタのコンソールは通常はシリアルデバイスを対象としています。
MONTSK(タスクモニタのタスク)の起動の引数で渡されるexinfの値をモニタの
通信出力デバイスとしています。但し、netDeviceのサーバとして起動する場合
コンソールはWindows上のnetTermとなります。

１−３．コンパイルスイッチ
タスクモニタのビルドで実行機能を指定するコンパイルスイッチについて説明します。

１）MONITOR_PORTID
モニタのデフォルトポートはシリアルコンソール（CONSOLE_PORTID）ですが、
モニタの起動コンソールを変更したい場合、コンパイルスイッチに
-DMONITOR_PORTID=nを指定すると変更が可能です。このとき、指定ポートのオープンは
モニタの起動時に行います。(Ver1.1.0のNEED_OPENのスイッチは削除します)

２）SUPPORT_PIPE
このスイッチを定義すると、外付けのコマンドを拡張します。
外付けコマンドは第1コマンド"PIPE"で起動します。
外付けコマンドの連絡関数はpipe_command(B *command);で設定してください。

３）SUPPORT_ETHER
TINETのコマンドサービス及びnetDeviceのコマンド拡張を行います。
このコマンドは第1コマンド"NET"で起動します。

４）START_TASKID
netDEviceの接続時、自動実行するタスク番号を指定します。
-DSTART_TASKID=1
のように指定すると、netDeviceの接続後、教材のプログラムが自動的に実行します。

５）NEED_MONITOR
このコンパイルスイッチを指定すると、モニタの起動時にneed_monitor()関数を呼び
出します。この関数の戻り値がFALSEの場合、モニタタスクを終了します。

６）MONITOR_DELAY
このコンパイルスイッチが指定された場合、モニタの起動時、コンソールの出力を
MONITOR_DELAY(ms)待ちを行った後で行います。

１−４．TOPPERS/JSPの実装拡張について
タスクモニタの以下の機能を実現するためにTOPPERS/JSPの実装依存configに
いくつかの改造が必要です。

１）_kernel_break_waitの定義の追加(必須)
JSPカーネルのサービスコールでの待ち状態の判定のために、_kernel_break_wait
ラベルをサービスコールの待ちプログラム位置に定義する必要があります。
サービスコールの待ちプログラム位置は通常各ＣＰＵのcpu_support.Sの何れかの
位置にあります。

２）iana_tskのリンクポイントの設定（必須ではない）
タスクモニタでは各タスクの実行履歴をdispatchのタスク切り替え時にiana_tsk
関数を呼び出すことで情報収集を行っています。そのためiana_tskの呼び出しに
より、スイッチング時間が大きく低下します。しかし、"log task"コマンドによる
タスク情報はリアルタイムシステムの検証には有用な手段です。
通常はコンパイルスイッチMONにより、取り外し可能とし、必要のない場合は
取り外して、使用してください。
dispatch関数は通常各ＣＰＵのcpu_support.S中にあります。

１−５．タスクモニタのマニュアル
１）displayコマンド
display byte [<start address> [<end address>]]
        <start address>から<end address>までのメモリまたはポート領域を
        １バイト単位に表示する。<end address>を省略した場合は128バイトの
        表示を行う。<start adress>を省略した場合は、前回表示を行った次の
        アドレスから表示を行う。

display half [<start address> [<end address>]]
        <start address>から<end address>までのメモリまたはポート領域を
        ２バイト単位に表示する。<end address>を省略した場合は128バイトの
        表示を行う。<start adress>を省略した場合は、前回表示を行った次の
        アドレスから表示を行う。

display word [<start address> [<end address>]]
        <start address>から<end address>までのメモリまたはポート領域を
        ４バイト単位に表示する。<end address>を省略した場合は128バイトの
        表示を行う。<start adress>を省略した場合は、前回表示を行った次の
        アドレスから表示を行う。

display task
        登録されたタスクの内容を表示する。表示内容は以下の通り。
        cur     	カレントタスクに*、モニタタスクはmonを表示する。
        id          タスクIDを表示する。
        pri         初期値のタスク優先度
        state       タスクの状態
        pc          RUN状態以外なら、現在のプログラムカウンタ値
        stack       現在のスタックアドレス
        inistack    初期値スタックの先頭アドレス
        inisize     初期値スタックのバイトサイズ

diaplay register
        カレントタスクが待ち状態となった場合、待ち状態のレジスタの内容を
        表示する。サービスコール中の待ちではレジスタの保存を行わない為、
        レジスタの表示は行わない。レジスタの内容はCPUによって異なる。

２）setコマンド
set byte [<start address>]
        <start address>の内容を１バイト表示し入力待ちとなる。１６進数入力後
        <RETURN>にてバイト値を入力すると<start address>に値を書込み後、
        <start address>の内容を再表示し<start address>+1の内容の入力待ちとなる
        .<RETURN>にてsetコマンドを終了しプロンプト表示に戻る。<start address>
        を省略した場合、最後に書込んだ次のアドレスの書込みを行う。

set half [<start address>]
        <start address>の内容を２バイト表示し入力待ちとなる。１６進数入力後
        <RETURN>にてバイト値を入力すると<start address>に値を書込み後、
        <start address>の内容を再表示し<start address>+2の内容の入力待ちとなる
        .<RETURN>にてsetコマンドを終了しプロンプト表示に戻る。<start address>
        を省略した場合、最後に書込んだ次のアドレスの書込みを行う。

set ward [<start address>]
        <start address>の内容を２バイト表示し入力待ちとなる。１６進数入力後
        <RETURN>にてバイト値を入力すると<start address>に値を書込み後、
        <start address>の内容を再表示し<start address>+2の内容の入力待ちとなる
        .<RETURN>にてsetコマンドを終了しプロンプト表示に戻る。<start address>
        を省略した場合、最後に書込んだ次のアドレスの書込みを行う。

set command [<mode>]
        コマンドモードを設定する。モードは１(1コマンド)、２(2コマンド)モードで
        デフォルトは２コマンドモード(例：displayとbyteの２つのコマンドで
        コマンド設定を行う)である。１コマンドは２コマンドの先頭の1文字を組み合
        わせた１つのコマンドでコマンド設定を行う。(例：display byteではdb)
        <mode>を省略すると現在のコマンドモードの表示を行う。
        ２コマンドモードでも、コマンドの最初の1文字の入力で実行可能である。
        (例：display byteでは d b)

set serial [<portid>]
        タスクモニタの入出力対応のシリアルのポート番号を<porid>で変更する
        <portid>を省略すると現在のポート番号を表示する。

set task [<task id>]
        <task id>で指定したタスクにカレントタスクを変更する。
        <task id>を省略するとモニタタスクをカレントタスクに設定する。

３）taskコマンド
task activate
        カレントタスクにact_tsk(<カレントタスクID>)サービスコールを発行する。

task terminate
        カレントタスクにter_tsk(<カレントタスクID>)サービスコールを発行する。

task suspend
        カレントタスクにsus_tsk(<カレントタスクID>)サービスコールを発行する。

task resume
        カレントタスクにrsm_tsk(<カレントタスクID>)サービスコールを発行する。

task priority [pir]
        カレントタスクにchg_tsk(<カレントタスクID>, <pri>)サービスコールを
        発行する。

４）logコマンド
log mode [<logmask> [<lowmask>]]
        syslogの表示モードを変更する。<logmask>と<lowmask>を省略した場合
        現在の<logmask>と<lowmask>を表示する。設定値のマスクの対応は
        以下の通りである。
        <マスク>   <設定値>
        LOG_EMERG     0     シャットダウンに値するエラー:lowmask default
        LOG_ALERT     1
        LOG_CRIT      2
        LOG_ERROR     3     システムエラー
        LOG_WARNING   4     警告メッセージ
        LOG_NOTICE    5                                 :logmask default
        LOG_INFO      6
        LOG_DEBUG     7     デバッグ用メッセージ
        logmask defaultとlowmask defaultは電源RTOS起動後のデフォルト値

log task [<cycle time>]
        <cycle time>ms周期に各タスクの前周期からのタスクの実行時間を表示する。
        <cycle time>を省略した場合、前回のlog task表示からの各タスクの
        実行時間を表示する。

log port [<no> <logno> [<portaddress>]]
        ポートモニタ機能の設定。<portaddress>を設定した場合<portaddress>が
        アクセスされるたびに<logno>にてアクセスデータをsyslogにてログ表示
        する。
         <no> <logno> <portaddress>の設定にて<portaddress>を<no>にlogmaskを
         <logno>にてポートモニタの設定を行う。但し、<logno>は1以上7までの値。
         <no> <0>の設定にてポートモニタの解除を行う。
         引数を省略すると、ポートモニタの登録状態の表示を行う。

５）volumeコマンド
TINETが有効であり、SUPPORT_VOLUMEコンパイルスイッチが有効の場合のみ使用可能な
コマンドである。
volume format <drive>
         <drive>に対してフォーマットを行う。

volume dir  [<path>]
         <path>のディレクトリを表示する。<path>を省略した場合、デフォルト
         deriveのディレクトリを表示する。

volume mkdir <path>
         <path>で指定したディレクトリを作成する。

volume rmdir <path>
         <path>で指定したディレクトリを削除する。

volume erase <path>
         <path>で指定したファイルを削除する。

６）pipeコマンド
SUPPORT_PIPEコンパイルスイッチが有効で、拡張コマンドが用意されている場合のみ
有効なコマンドであり、コマンド仕様は拡張されたユーザ設定コマンドの仕様に
従う、セカンドコマンドがユーザ設定コマンドとなる。

７）helpコマンド
モニタコマンドのヘルプ情報を表示する。

２．モニタの入出力に関して
モニタのデータ取り込みは、(FILE*)mon_infileに対するデータ取り込みを行う
モニタのデータ送信は、標準入出力の出力ファイル(stdout)に対して行う。


３．標準入出力の対応

３−１．解説
標準入出力はnewlib版と簡易版を選択できます。
newlib版は簡易版に比べ、H8のサイズで32KBほど大きくなります。
状況に応じて、選択してください。
newlib版はh8の環境のみ実行確認を行っています。他の環境では
実行確認を行っていません。


３−２．簡易版標準入出力
monitorデレクトリィ中の以下のファイルを使用します
　stdio.h　　（newlib中のstdio.hに優先して参照する）
  printf.c　　(printf文)
  sprintf.c   (sprintf文)
サポートしている標準入出力関数はstdio.hを参照してください
また、printf関数やsprintf関数がlibc.aに優先してリンクされるように
ライブラリィより前に、printf.oとsprintf.oを取り込むようにしてください。
また、OAKS16用の開発環境TMで使用する場合はstdio.hを削除しMTOOLの
stdio.hを使用するようにしてください。

３−３．newlib版標準入出力
monitorデレクトリィ中のstdio.h、stddev.c、printf.c、sprintf.cを参照しない
ように、これらのファイルを削除してください。
また、monitor/Makefile.config中からstddev.o、printf.o、sprintf.oをビルド時
にリンクしないように削除してください。
newlib中の標準入出力は、putcharやputcのような一文字出力関数は\n(改行)コードが
ないと表示を行いません。また、printfやputsのような文字列出力関数は\nコードが
付けられて出力されます。
putcharやputcの対応を行うには、初期化としてバッファのない指定を行えば
解決できます。この教材ではnewlibrc.cを改造し、タスクモニタの初期化時、
setbuf(stdio, 0);関数にて、出力バッファなしの指定を行っています。
タスクモニタを使用しない場合は、setbufの設定を自分で行ってください。
printfやputsのような文字列の出力関数に関しては、newlib中のputs.cを
修正することにより、これを回避することが出来ます。
詳細は３．newlibの改造を参照してください。

ライブラリィのビルド手順は以下の通りです。
/home/roi/gnuにnewlibの凍結ファイルをコピーし
gnu>tar zxvf newlib-1.11.0.tar.gz
gnu>mkdir /usr/local/h8300-hms
gnu>mkdir newlib-objdir
gnu>cd newlib-objdir
gnu>export TARGET_CFLAGS=-DREENTRANT_SYSCALLS_PROVIDED
gnu>/home/roi/gnu/newlib-1.11.0/configure --target=h8300-hms --prefix=/usr/local/h8300-hms
gnu>make
gnu>make install

アプリケーションのビルド時、標準入出力用の実装部として改造した
systask/newlibrt.cをリンクする必要があります。


３−４．newlibの改造(newlib-1.11.0/newlib/libc/stdio/)
puts.cの修正部
/*
 * Write the given string to stdout, appending a newline.
 */

int
_DEFUN (_puts_r, (ptr, s),
	struct _reent *ptr _AND
	_CONST char * s)
{
  size_t c = strlen (s);
  struct __suio uio;
  struct __siov iov[2];

  iov[0].iov_base = s;
  iov[0].iov_len = c;
#if 0	/* TOPPERS.JSP */
  iov[1].iov_base = "\n";
#else	/* TOPPERS.JSP */
  iov[1].iov_base = "\0";
#endif	/* TOPPERS.JSP */
  iov[1].iov_len = 1;
  uio.uio_resid = c + 1;
  uio.uio_iov = &iov[0];
  uio.uio_iovcnt = 2;

  _REENT_SMALL_CHECK_INIT(_stdout_r (ptr));
  return (__sfvwrite (_stdout_r (ptr), &uio) ? EOF : '\n');
}


また、標準入出力用のローレベル関数はsystask/newlibrt.cに記載してあります。
既存のnewlibrt.cはnewlib用のC言語ライブラリィとC++言語ライブラリィと
初期化プログラムがいっしょに記述してありましたので以下の２つのソースに
分けました。newlibの標準入出力を使用する場合は新しいnewlibrt.cを
プログラムのビルド時にリンクするようにしてください。
・新しいnewlibrt.c：C言語用のnewlibライブラリィのTOPPERSとのI/F部
・cpplibrt.c：C++言語用の関数と初期化プログラム

