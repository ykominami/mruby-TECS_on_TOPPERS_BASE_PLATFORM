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


### 使用モジュールの展開

使用モジュールは、本リポジトリにすべて展開済みです。

### 現在の状況

1) 初期チェックイン asp_baseplatformv1.3.0, mruby-3.0.0
1) 追加チェックイン asp-1.9.3, asp_arch_arm_m7_gcc, TLSF-2.4.6
1) 追加チェックイン tecsgen-1.7.0


