source ./configure-lib.sh

variable builddir build


# ---------------------------------------------------------------------------------------------------------------------

executable lemon tool/lemon.c

rule lemon --implicit "\$builddir/lemon" --implicit "tool/lempar.c" --implicit "tool/grammar_prefix.y" --command "mkdir -p \$builddir/\$out; cat tool/grammar_prefix.y \$in > \$builddir/\$out/grammar.y; \$builddir/lemon \$builddir/\$out/grammar.y -Ttool/lempar.c -l -d\$builddir/\$out; cp \$builddir/\$out/grammar.c \$out"

rule re2c --command "re2c \$in -o \$out"

rule lemon_re2c --implicit "\$builddir/lemon" --implicit "tool/lempar.c" --implicit "tool/grammar_prefix.y" --command "mkdir -p \$builddir/\$out; cat tool/grammar_prefix.y \$in > \$builddir/\$out/grammar.y; \$builddir/lemon \$builddir/\$out/grammar.y -Ttool/lempar.c -l -d\$builddir/\$out; re2c \$builddir/\$out/grammar.c -o \$out -f --storable-state"

# ---------------------------------------------------------------------------------------------------------------------

build build/fileformat_configuration_parser.c lemon_re2c gen/fileformat_configuration_parser.c.re2c.lemon

# ---------------------------------------------------------------------------------------------------------------------

per_platform library ___-libuv --pkg-config libuv

library lnx-glfw3 --platform lnx --pkg-config glfw3 --lflag -lGL
library win-glfw3 --platform win --pkg-config glfw3 --lflag -lgdi32 --lflag -lssp

per_platform executable ___-city_of_shadows \
  ___-libuv \
  ___-glfw3 \
  --define "IMPLEMENTATION_LIBUV=1" \
	--cflag -g \
  build/fileformat_configuration_parser.c \
  src/configuration.c \
  src/display.c \
  src/ecs_archetype.c \
  src/ecs.c \
  src/ecs_component.c \
  src/ecs_entity.c \
  src/ecs_layer.c \
  src/ecs_memory.c \
  src/format.c \
  src/log.c \
  src/main.c \
  src/memory.c \
  src/platform.c \
  src/read.c \
  src/render.c \
  src/resource.c \
  src/script.c \
  src/sort.c \
  src/string.c \
  src/vfs.c \
  src/write.c

# ---------------------------------------------------------------------------------------------------------------------

rule configure --command "./configure.sh > build.ninja" --implicit "configure-lib.sh configure.sh" --generator
build build.ninja configure

