#!/bin/bash -e

pushd "$1" > /dev/null
diff -U1 <(grep -o 'quirks/.*\.quirks' meson.build) <(ls quirks/*.quirks)
popd > /dev/null
