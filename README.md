# mruby+TECS on TOPPERS BASE PLATFORM
porting mruby on TOPPERS BASE PLATFORM using TECS

TECS を使って mruby を TOPPERS BASE PLATFORM に移植します。

TECS を使うと、驚くほど簡単に移植できます。
コンフィグレーションも簡単です。

## ターゲット

以下のターゲットデバイスを使用します。

   **SMT32F746 Discovery Kit**

TOPPERS BASE PLATFORM は、各種の ST 社のボードに対応しており、これ以外のデバイスでも、同様にできるはずです。  
ROM 256KB, RAM 256KB 以上が必要です。RAM 実行する場合は、RAM 512KB 以上が必要です。

mruby VM の実装、tSampleMruby の実装までは、TOPPERS BASE PLATFORM 以外の TOPPERS カーネルにも同様に実装できます。

## 使用モジュール

以下は、mruby+TECS on TOPPERS BASE PLATFORM を構成するモジュールです。
このリポジトリに置かれています。

* [TOPPERS/ASP ターゲット非依存部](https://www.toppers.jp/asp-d-download.html)  ([asp-1.9.3.tar.gz](https://www.toppers.jp/download.cgi/asp-1.9.3.tar.gz))
* [TOPPERS/ASP ターゲット依存部](https://www.toppers.jp/asp-d-download.html)  ([asp_arch_arm_m7_gcc-1.9.5.tar.gz](https://www.toppers.jp/download.cgi/asp_arch_arm_m7_gcc-1.9.5.tar.gz))
* [TOPPERS BASE PLAFORM(ST)](https://www.toppers.jp/edu-baseplatform.html#st) ([asp_baseplatformv1.3.0_052018.tar.gz](https://www.toppers.jp/download.cgi/asp_baseplatformv1.3.0_052018.tar.gz))
* [mruby 3.0.0](https://mruby.org/releases/2021/03/05/mruby-3.0.0-released.html)  ([mruby-3.0.0.tar.gz](https://github.com/mruby/mruby/releases/tag/3.0.0))
* [TLSFメモリアロケータ](http://www.gii.upv.es/tlsf/) ([TLSF-2.4.6](http://wks.gii.upv.es/tlsf/files/src/TLSF-2.4.6.tbz2))
* [TECS ジェネレータ](https://www.toppers.jp/tecs.html) ([tecsgen-1.7.0.tgz](https://www.toppers.jp/download.cgi/tecsgen-1.7.0.tgz))

## 使用ツール類

Windows 10 でビルドする場合に必要となるツール類です。  
Linux や MacOS でビルドする場合には、それらようの環境に合わせて、適切なものをご準備ください。

* [cygwin](https://www.cygwin.com/)
* [arm 用 gcc コンパイラ](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) ([gcc-arm-none-eabi-9-2020-q2-update-win32.exe](https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-win32.zip?revision=95631fd0-0c29-41f4-8d0c-3702650bdd74&hash=60FBF84A2ADC7B1F508C2D625E831E1F1184F509))
* [ST-LINK Utility](https://www.st.com/content/st_com/ja/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html)
* [Tera Term](https://ja.osdn.net/projects/ttssh2/) ([teraterm-4.106](https://ja.osdn.net/projects/ttssh2/downloads/74780/teraterm-4.106.exe/))
* [TOPPERS新世代カーネル用コンフィグレータ](https://www.toppers.jp/cfg-download.html) ([cfg-1.9.6](https://www.toppers.jp/download.cgi/cfg-mingw-static-1_9_6.zip))


使用モジュールは、本リポジトリにすべて展開済みですので、チェックアウトして、そのままビルドできます。

asp-1.9.3.tar.gz, asp_arch_arm_m7_gcc-1.9.5.tar.gz, asp_baseplatformv1.3.0_052018.tar.gz, mruby-3.0.0.tar.gz は、開発用ルートディレクトリで、そのまま解凍します。

TLSF-2.4.6.tbz2 は、asp ディレクトリ下で解凍してください。

tecsgen-1.7.0.tgz は、解凍後 tecsgen-1.7.0/tecsgen ディレクトリを asp ディレクトリ下へ移動してください。移動後、残ったものは削除してください。

使用モジュールに含めていませんでしたが、後述のとおりTECS 簡易パッケージから tecs_kernel ディレクトリを持ってきています。さらに syssvc ディレクトリから必要なファイルを抜き出して持ってきています。

## TECS 化の方針

TOPPERS BASE PLATFORM で使用している TOPPERS/ASP カーネルは、公式リリースとしては TECS に対応していません。

TECS WG から [TECS 簡易パッケージ](https://www.toppers.jp/tecs_archive.html) として公開されていますが、メンテナンスれていません。

このため、TOPPERS/ASP を TECS 化する必要があります。

以下の方針で TECS 化します。

* kernel.cdl および、そこに定義されているセルタイプのセルタイプコードは、TECS 簡易パッケージに含まれているものを使用する

* tSerial, tSysLog は、標準の TOPPERS/ASP のラッパーとして実装する (TECS簡易パッケージに含まれているものは TOPPERS/ASP3 と同様に、フルに TECS 化されている)

余談ですが TOPPERS/ASP簡易パッケージ、 TOPPERS/ASP3 で tSysLog がフルに TECS 化されているのは、ターゲットによりカスタマイズされることが多いため、可変点として、その部分を切り出してコンポーネント化がなされました。

## 進め方

以下の4つについて Step by step で進めていきます。

 1. 非 TECS 版 sample1 のビルド
 2. 非 TECS 版 sample1 を ROM 化対応してビルド
 2. mruby VM を実装してのビルド
 3. TECS 版 tSample1 のビルド
 4. mruby で tSample1 の実現

 ## 非 TECS 版 sample1 のビルド

 非 TECS 版の sample1 は、TOPPERS/ASP の標準的なサンプルです。本題では、ありませんが、まずは基本形ができていることが重要ですので、ここからスタートです。

     % cd asp/
     % mkdir -p OBJ_MRUBY_TECS/STM32F7DISCOVERY_GCC/a_sample1
     % ../../../configure -T stm32f7discovery_gcc
     % make

これで asp.srec ができましたら ROM モニタ上で実行できます。
ROM モニタの書き込み方法は、TOPPERS BASE PLATFORM(asp_baseplatformv1.3.0_052018.tar.gz) に同梱の「ＴＯＰＰＥＲＳ基礎実装セミナー （STM32F4-Discovery版：基本）　開発環境」(BaseTrainingSeminar_environment-020010002.pdf) の ROM モニタの書き込みを参照してください。

実行には Tera Term を起動して出力を確認します。これも上記の資料に詳しく説明されています。

## 非 TECS 版 sample1 を ROM 化対応してビルド

SMT32F746 Discovery Kit の CPU ローカルな RAM は 320KB しかありません。このため mruby VM を実装すると RAM にロードしてデバッガ実行することができません。

このため、ROM に焼いて実行するようにします。まずは、Makefile を修正してビルドしなおす必要があります。

RAM 動作版とは、別の環境でビルドすることにします。RAM 動作版と同じディレクトリから始めることとします。まずは、コンフィグレータまで、実行します。

     % cd asp/OBJ_MRUBY_TECS/STM32F7DISCOVERY_GCC
     % mkdir b_sample1_rom
     % ../../../configure -T stm32f7discovery_gcc

Makefile の以下の行を変更する必要があります。

修正前

    DBGENV :=

修正後

    DBGENV := ROM

Makefile の修正が終わりましたら、ビルドします。

    % make

念のため期待したようにビルドされているか確認しましょう。確認するには asp.syms を参照します。以下のように text領域 (機械語命令が置かれる領域) が 0x08000000 番地から始まっていれば成功です。

    08000000 T __text

これは ROM の開始アドレスですが、使用する CPU やボードに依存します。RAM 実行版では RAM の開始アドレスである 0x20000000 になっていましたので、併せて確認してみてください。

##  mruby VM のビルド

先に mruby をビルドしておきます。
ホスト環境用と、ARM 用をビルドします。

### ホスト環境用をビルド

初めにホスト用の mruby をビルドします。
これは、mruby コンパイラ mrbc を使用するため必要となります。

    % cd mruby-3.0.0
    % make

### ビルドスクリプトの変更

arm 用 mruby をビルドする前に、ビルドスクリプトを少し変更します。

mruby のビルドスクリプトを実行すると、コンパイラを呼び出すときに、絶対パスでファイル名を指定するように組まれています。

今回使用する ARM コンパイラは、非 cygwin のものであるため、このことが問題になります。
相対パスであれば、問題ありません。

本格的な修正案は、ykominami さんが、[こちら](https://github.com/ykominami/mruby) で相対パスを指定する拡張を公開してくだっていますが、ここでは安直に相対パスを指定するように変更します。

[mruby-3.0.0/lib/mruby/build.rb](https://github.com/hiro22022/mruby-TECS_on_TOPPERS_BASE_PLATFORM/blob/main/mruby-3.0.0/lib/mruby/build.rb) の filename メソッドを変更します。

    変更前)   # name.gsub('/', file_separator)    
    変更後)   name.relative_path.gsub('/', file_separator)

### ARM 用 mruby のビルド

TOPPERS BASE PLATFORM に合わせたクロスビルド用のスクリプト
[mruby-3.0.0/build_config/toppers_arm_m7.rb](https://github.com/hiro22022/mruby-TECS_on_TOPPERS_BASE_PLATFORM/blob/main/mruby-3.0.0/build_config/toppers_arm_m7.rb)
を用意します。

コンパイラのオプションは、Makefile.target, Makefile.chip で指定されているものと合わせる必要があります。
合っていなくても、リンク時にエラーにならないので、ビルド後に動作しない場合、チェックすべき一つになります。

ビルドコマンドは、以下の通りです。rake コマンドにクロスビルド用のスクリプトを指定します。

    % rake MRUBY_CONFIG=toppers_arm_m7

## TECS BASE PLATFORM へ mruby VM を組み込む

いよいよ本題へ入っていきます。TOPERS BASE PLATFORM に mruby VM を組込みます。
これをするために、既存の ROM 化した sample1 をベースに開発を進めます。
ROM 化した sample1 を、そのままコピーしてから始めます。

     % cd asp/OBJ_MRUBY_TECS/STM32F7DISCOVERY_GCC
     % cd b_sample1_rom
     % make clean
     % cd ..
     % mkdir c_mruby
     % cd c_mruby
     % cp * ../c_mruby

### 修正ファイル

修正が必要なものは、以下のファイルです。

 * Makefile
 * sample1.cfg
 * nMruby_tMrubyVM.c
 * tMruby.cdl
 * target_stddef.h

以下、簡単に修正内容について説明します。

#### Makefile の変更

Makefile の修正箇所は、たくさんありますが、変更の主旨は、以下の3点です。

 * TECS ジェネレータ実行および TECS 関係のオブジェクトのリンク
 * mruby へのパスと libmrby.a のリンク
 * TLSF へのパスとオブジェクトのリンク

#### sample1.cfg の変更

TECS ジェネレータにより生成される cfg ファイルを取り込みます。

 * tecsgen.cfg の INCLUDE

#### nMruby_tMrubyVM.c の変更

 TECS ジェネレータ V1.7.0 に同梱されている nMruby_tMrubyVM.c は mruby 3.0.0 に対応しません。
 この点を修正します。

 また、nMruby_tMrubyVM.c に mruby が参照する API で、必要のないもののダミー定義が
 このファイルに記載されていました。
 これは実装により変わるものですので、ここへダミー定義を入れておくのは
 汎用性の観点でよろしくありませんので、ここから外しました。

#### tMruby.cdl

依存関係を tecs.timestamp から $(GEN_DIR)/tecsgen.timestamp に変更しています。
この変更は TECS ジェネレータ(tecsgen) V1.3.1.0 であれば影響しません。

#### target_stddef.h

target/stm32f7discovery_gcc/target_stddef.h で stdint.h をインクルードするようにします。
さもないとビルド時にエラーが発生するようになります。

修正前

     #define TOPPERS_STDINT_TYPE1

修正後

     // #define TOPPERS_STDINT_TYPE1
     #ifndef TOPPERS_MACRO_ONLY
     #include <stdint.h>
     #endif // TOPPERS_MACRO_ONLY


### 追加ファイル

以下のファイルを追加します。

 * my_mruby.cdl
 * my_mruby.rb
 * tMrubyStarter.c
 * dummy.c

以下、簡単に内容を説明します。

#### my_mruby.cdl 

TECS コンポーネント記述言語 （TECS CDL）による記述です。

mruby VM が動作するタスク (MrubyTas)、mruby VM (Mruby) が定義されています。
タスクのスタックサイズや優先度、ヒープサイズなど調整すべき要素が、この CDL ファイルに集約されています。また、セルが定義されているセルタイプのみ、コンパイル、およびリンクされるように Makefile が調整されますので、cfg ファイル、Makefile を触る必要がなくなります。

MrubyStarter は、開始終了メッセージを syslog に出力するものですが、必須ではありません。
MrubyTask の結合先を変更して、MrubyStarter セルの定義をコメントアウトして
ビルドしなおすと、完全に取り除くことができます。
tMrubyVMStarter セルタイプの定義は、残しておいてもリンクされることはありません。
リンクするオブジェクト (.o) は自動的に調整されます。

#### my_mruby.rb

これは mruby のスクリプトです。
この例では、mruby が無事に動作したことを確認するために
メッセージを表示するものとなっています。

#### tMrubyStarter.c

少しスリープした後、メッセージを出力したのち、mruby VM を呼び出します。

#### dummy.c

mruby VM は、組込み用と言っても組込み Linux のような POSIX 環境が前提のようです。
dummy.c では、不足する API のダミー関数を定義しています。
これらの API は、環境によっては提供される可能性がありますので、
nMruby_tMrubyVM.c から外してあります。

### ビルド

ビルドは、以下のコマンドで実行できます。

    % make

my_mruby.rb は、mrbc コマンドによりコンパイルされて
バイトコード (mruby 仮想マシンの機械語) に変換されたものが
リンクされます。

ビルドに成功したら **SMT32F746 Discovery Kit** の ROM に書き込んで
実行してみましょう。
Tera Term に "Welcome to mruby & TECS" と表示されたら成功です。

## 現在の状況

1) 初期チェックイン asp_baseplatformv1.3.0, mruby-3.0.0
1) 追加チェックイン asp-1.9.3, asp_arch_arm_m7_gcc, TLSF-2.4.6
1) 追加チェックイン tecsgen-1.7.0
1) 方針、進め方まで記載
1) 非 TECS 版 sample1 のビルド
1) 非 TECS 版 sample1 を ROM 化対応してビルド
1) mruby VM のビルド
1) TECS BASE PLATFORM へ mruby VM を組み込む

