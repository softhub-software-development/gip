
POF=../../wrk
SRC=${POF}/src
DST=.

declare -a DIRS=(
  ${DST}/build/Linux/*.*
  ${DST}/build/Linux/gip/*.*
  ${DST}/build/OSX/GeoIP/*.*
  ${DST}/src/application/*.*
  ${DST}/src/application/gip/*.*
  ${DST}/src/common/*.*
  ${DST}/src/common/base/*.*
  ${DST}/src/common/hal/*.*
  ${DST}/src/common/net/*.*
  ${DST}/src/library/*.*
  ${DST}/src/library/geography/*.*
  ${DST}/src/library/util/*.*)

for f in ${DIRS[@]}
do
  cp -rv ${POF}/${f} ${f}
done

