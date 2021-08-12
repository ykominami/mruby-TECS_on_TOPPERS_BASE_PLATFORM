# Cross Compiling configuration for TOPPERS BASE PLATFORM ARM Cortex-M7
#
MRuby::CrossBuild.new("toppers_arm_m7") do |conf|
  toolchain :gcc

  [conf.cc].each do |cc|
    cc.command = ENV['CC'] || 'arm-none-eabi-gcc'
    cc.flags = [ENV['CFLAGS'] || %w(-mlittle-endian -nostartfiles -mthumb  -mcpu=cortex-m7 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Wall -O2)]
    cc.include_paths = ["#{MRUBY_ROOT}/include", "../asp/include", "../asp/target/stm32f7discovery_gcc/", "../asp/arch"],

    cc.defines = ["DISABLE_GEMS", "ev3rt"] # %w(DISABLE_GEMS)
    cc.option_include_path = '-I%s'
    cc.option_define = '-D%s'
    cc.compile_options = '%{flags} -MMD -o %{outfile} -c %{infile}'
  end

  conf.cxx do |cxx|
    cxx.command = conf.cc.command.dup
    cxx.include_paths = conf.cc.include_paths.dup
    cxx.flags = conf.cc.flags.dup
    cxx.flags << %w(-fno-rtti -fno-exceptions)
    cxx.defines = conf.cc.defines.dup
    cxx.compile_options = conf.cc.compile_options.dup
  end

  conf.linker do |linker|
    linker.command = ENV['LD'] || 'arm-none-eabi-gcc'
    linker.flags = [ENV['LDFLAGS'] || %w()]
    linker.libraries = %w(m)
    linker.library_paths = []
    linker.option_library = '-l%s'
    linker.option_library_path = '-L%s'
    linker.link_options = '%{flags} -o %{outfile} %{objs} %{flags_before_libraries} %{libs} %{flags_after_libraries}'
  end

  conf.archiver do |archiver|
    # archiver.command = "#{BIN_PATH}/arm-none-eabi-ar"
    archiver.command = "arm-none-eabi-ar"
    archiver.archive_options = 'rcs "%{outfile}" %{objs}'
  end

  #no executables
  conf.bins = []

  #do not build executable test
  conf.build_mrbtest_lib_only

  #disable C++ exception
  conf.disable_cxx_exception

=begin
  #gems from core
  conf.gem :core => "mruby-print"
  conf.gem :core => "mruby-math"
  conf.gem :core => "mruby-enum-ext"
=end

  # Use standard print/puts/p
  conf.gem :core => "mruby-print"
  # Use extended toplevel object (main) methods
#  conf.gem :core => "mruby-toplevel-ext"
  # Use standard Math module
  #  conf.gem :core => "mruby-math"
  # Use mruby-compiler to build other mrbgems
#  conf.gem :core => "mruby-compiler"
#  conf.gem :core => "mruby-array-ext"

  #light-weight regular expression
#  conf.gem :github => "masamitsu-murase/mruby-hs-regexp", :branch => "master"

end
