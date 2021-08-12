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

### 現在の状況

1) 初期チェックイン asp_baseplatformv1.3.0, mruby-3.0.0
1) 追加チェックイン asp-1.9.3, asp_arch_arm_m7_gcc, TLSF-2.4.6
1) 追加チェックイン tecsgen-1.7.0
1) 方針、進め方まで記載

