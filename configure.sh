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

executable geometric_algebra tool/geometric_algebra.c

rule gen_geometric_algebra --implicit \$builddir/geometric_algebra --command "\$builddir/geometric_algebra \$flags > \$out"

build "\$builddir/include/generated/pga2d.h" gen_geometric_algebra "" --flags "-p 2 -d 1 --binary meet outer_product --binary join regressive_product --code m02"
build "\$builddir/include/generated/pga3d.h" gen_geometric_algebra "" --flags "-p 3 -d 1 --binary meet outer_product --binary join regressive_product --code m024"
build "\$builddir/include/generated/cga2d.h" gen_geometric_algebra "" --flags "-p 3 -n 1 --binary meet outer_product --binary join regressive_product"
build "\$builddir/include/generated/cga3d.h" gen_geometric_algebra "" --flags "-p 4 -n 1 --binary meet outer_product --binary join regressive_product"

phony geometric_algebra_headers \
  --include-directory "\$builddir/include" \
  "\$builddir/include/generated/pga2d.h" \
  "\$builddir/include/generated/pga3d.h" \
  "\$builddir/include/generated/cga2d.h" \
  "\$builddir/include/generated/cga3d.h"

per_platform executable ___-city_of_shadows \
  ___-libuv \
  ___-glfw3 \
  --define "IMPLEMENTATION_LIBUV=1" \
	--cflag -g \
	--lflag -lm \
	geometric_algebra_headers \
  build/fileformat_configuration_parser.c \
  src/camera.c \
  src/configuration.c \
  src/display.c \
  src/draw.c \
  src/draw_screen.c \
  src/ecs_archetype.c \
  src/ecs.c \
  src/ecs_component.c \
  src/ecs_entity.c \
  src/ecs_layer.c \
  src/ecs_memory.c \
  src/ecs_query.c \
  src/font.c \
  src/font-breeserif.c \
  src/format.c \
  src/game.c \
  src/game_body_capacity.c \
  src/game_body_parts.c \
  src/game_map.c \
  src/game_need.c \
  src/game_skill.c \
  src/game_terrain.c \
  src/gl.c \
  src/image.c \
  src/input.c \
  src/log.c \
  src/main.c \
  src/memory.c \
  src/physics.c \
  src/platform.c \
  src/read.c \
  src/render.c \
  src/resource.c \
  src/script.c \
  src/sort.c \
  src/string.c \
  src/transform.c \
  src/ui.c \
  src/vfs.c \
  src/write.c

# ---------------------------------------------------------------------------------------------------------------------

rule configure --command "./configure.sh > build.ninja" --implicit "configure-lib.sh configure.sh" --generator
build build.ninja configure

