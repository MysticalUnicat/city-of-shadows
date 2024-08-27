#!/bin/bash

variable() {
  local name=$1
  shift
  echo "$name = $*"
}

declare -A rule_implicit_dependencies

rule() {
  local name=$1
  shift

  local command=
  local deps=
  local depfile=
  local implicit_dependencies=
  local generator=

  while (( "$#" )); do
    local arg=$1
    shift
    if [[ $arg = --command ]]; then
      command=$1
      shift
    elif [[ $arg = --generator ]]; then
      generator=1
    elif [[ $arg = --implicit ]]; then
      implicit_dependencies="$implicit_dependencies $1"
      shift
    elif [[ $arg = --deps ]]; then
      deps="$1"
      shift
    elif [[ $arg = --depfile ]]; then
      depfile="$1"
      shift
    fi
  done

  rule_implicit_dependencies[$name]=$implicit_dependencies

  echo "rule $name"
  if [[ ! -z $command ]]; then
    echo "  command = $command"
  fi
  if [[ ! -z $deps ]]; then
    echo "  deps = $deps"
  fi
  if [[ ! -z $depfile ]]; then
    echo "  depfile = $depfile"
  fi
  if [[ ! -z $generator ]]; then
    echo "  generator = 1"
  fi
}

build() {
  local output=$1
  shift

  local _rule=$1
  shift

  local inputs=
  local implicit_dependencies=${rule_implicit_dependencies[$_rule]}
  local flags=

  while (( "$#" )); do
    local arg=$1
    shift
    if [[ $arg = --implicit ]]; then
      implicit_dependencies="$implicit_dependencies $1"
      shift
    elif [[ $arg = --flags ]]; then
      flags=$1
      shift
    else
      inputs="$inputs $arg"
    fi
  done

  implicit_dependencies=$(echo $implicit_dependencies | uniq)

  if [[ ! -z "$implicit_dependencies" ]]; then
    implicit_dependencies="| $implicit_dependencies"
  fi

  echo "build $output : $_rule $inputs $implicit_dependencies"
  if [[ ! -z "$flags" ]]; then
    echo "  flags = $flags"
  fi
}

PKG_CONFIG_lnx=pkg-config
CC_lnx=gcc
LINK_lnx=gcc
EXE_lnx=
CFLAGS_lnx=
LFLAGS_lnx=
AR_lnx=ar

PKG_CONFIG_win=x86_64-w64-mingw32-pkg-config
CC_win=x86_64-w64-mingw32-gcc
LINK_win=x86_64-w64-mingw32-gcc
EXE_win=".exe"
CFLAGS_win=
LFLAGS_win=
AR_win=ar

declare -A target_defined target_include_directories target_defines target_cflags target_lflags target_implicit_dependencies target_link_dependencies

target() {
  local name=$1
  shift

  local platform=lnx
  local sources=
  local objects=
  local executable=
  local phony=
  local include_directories=
  local defines=
  local private_defines=
  local cflags=
  local lflags=
  local implicit_dependencies=
  local link_dependencies=

  local no_c=

  local PKG_CONFIG=PKG_CONFIG_${platform}

  while (( "$#" )); do
    local arg=$1
    shift

    if [[ $arg = --executable ]]; then
      executable=1
    elif [[ $arg = --phony ]]; then
      phony=1
    elif [[ $arg = --include-directory ]]; then
      include_directories="$include_directories -I$1"
      shift
    elif [[ $arg = --cflag ]]; then
      cflags="$cflags $1"
      shift
    elif [[ $arg = --lflag ]]; then
      lflags="$lflags $1"
      shift
    elif [[ $arg = --implicit ]]; then
      implicit_dependencies="$implicit_dependencies $1"
      shift
    elif [[ $arg = --define ]]; then
      defines="$defines -D$1"
      shift
    elif [[ $arg = --define-private ]]; then
      private_defines="$private_defines -D$1"
      shift
    elif [[ $arg = --pkg-config ]]; then
      PKG_CONFIG=PKG_CONFIG_${platform}
      PKG_CONFIG_CMD="${!PKG_CONFIG}"
      out_cflags=$($PKG_CONFIG_CMD --cflags $1)
      out_lflags=$($PKG_CONFIG_CMD --libs $1)
      cflags="$cflags $out_cflags"
      lflags="$lflags $out_lflags"
      shift
    elif [[ $arg = --platform ]]; then
      platform=$1
      shift
    else
      local tgt=${target_defined[$arg]}
      if [[ -z "$tgt" ]]; then
        if [[ -z "$phony" ]]; then
          sources="$sources $arg"
          no_c=${arg%.c}
          [ $arg = $no_c ] && echo "only c sources can be used, got $arg"
          objects="$objects \$builddir/${platform}-${no_c//\//-}.o"
        else
          phony="$phony $arg"
        fi
      else
        include_directories="$include_directories ${target_include_directories[$arg]}"
        defines="$defines ${target_defines[$arg]}"
        cflags="$cflags ${target_cflags[$arg]}"
        lflags="$lflags ${target_lflags[$arg]}"
        implicit_dependencies="$implicit_dependencies ${target_implicit_dependencies[$arg]}"
        link_dependencies="$link_dependencies ${target_link_dependencies[$arg]}"
      fi
    fi
  done

  local PKG_CONFIG=PKG_CONFIG_${platform}
  local CC=CC_${platform}
  local LINK=LINK_${platform}
  local EXE=EXE_${platform}
  local CFLAGS=CFLAGS_${platform}
  local LFLAGS=LFLAGS_${platform}
  local AR=AR_${platform}

  if [[ ! -z "$phony" ]]; then
    implicit_dependencies="$implicit_dependencies $name"

    build $name phony ${phony:1}
  fi

  implicit_dependencies=$(echo $implicit_dependencies | uniq)

  if [[ ! -z "$sources" ]]; then
    RULE_DEFINES=$(echo $defines $private_defines | uniq)
    
    rule ${name}_cc \
      --command "${!CC} -MD -MF \$out.d $include_directories $RULE_DEFINES ${!CFLAGS} $cflags -c \$in -o \$out" \
      --depfile "\$out.d" \
      --deps "gcc"

    for src in $sources; do
      no_c=${src%.c}
      build "\$builddir/${platform}-${no_c//\//-}.o" ${name}_cc $src --implicit "$implicit_dependencies"
    done

    if [[ ! -z "$executable" ]]; then
      # ⍷⌾⌽ in BQN. Deduplicate _under_ Reverse
      cleaned_lflags=$(tr ' ' '\n' <<<"$lflags" | tac | awk '!u[$0]++' | tac | tr '\n' ' ')
      cleaned_link_dependencies=$(tr ' ' '\n' <<<"$link_dependencies" | tac | awk '!u[$0]++' | tac | tr '\n' ' ')

      rule ${name}_link \
        --command "${!LINK} ${!CFLAGS} $cflags \$in ${!LFLAGS} -L\$builddir/${platform}-lib $cleaned_lflags -o \$out"

      build \$builddir/$name${!EXE} ${name}_link $objects --implicit "$cleaned_link_dependencies"
    else
      rule ${name}_ar \
        --command "rm -f \$out && ${!AR} crs \$out \$in"

      build \$builddir/${platform}-lib/lib${name}.a ${name}_ar $objects

      lflags="-l$name $lflags" # break append convention so libraries are linked from child to parent
      link_dependencies="$link_dependencies \$builddir/${platform}-lib/lib${name}.a"
    fi
  fi

  target_defined[$name]=1
  target_include_directories[$name]=$(echo $include_directories | uniq)
  target_defines[$name]=$(echo $defines | uniq)
  target_cflags[$name]=$(echo $cflags | uniq)
  target_lflags[$name]=$(echo $lflags | uniq)
  target_implicit_dependencies[$name]=$implicit_dependencies
  target_link_dependencies[$name]=$link_dependencies
}

phony() {
  local name=$1
  shift
  target $name --phony $*
}

library() {
  target $*
}

executable() {
  local name=$1
  shift
  target $name --executable $*
}

PLATFORMS=( lnx win )

per_platform_0() {
  local platform=$1
  shift

  local cmd=$1
  shift

  local name=$1
  shift

  if [[ $name == ___-* ]]; then
    name="${name:4}"
  fi

  local args=

  while (( "$#" )); do
    local arg=$1
    shift

    arg=$(echo $arg | sed "s/___/$platform/")

    args="$args $arg"
  done

  $cmd $platform-$name --platform $platform $args
}

per_platform() {
  for PLATFORM in "${PLATFORMS[@]}"
  do
    per_platform_0 $PLATFORM $@
  done
}

