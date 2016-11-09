#!/bin/bash
# create multiresolution windows icon
ICON_DST=../../src/qt/res/icons/dopecoin.ico

convert ../../src/qt/res/icons/dopecoin-16.png ../../src/qt/res/icons/dopecoin-32.png ../../src/qt/res/icons/dopecoin-48.png ${ICON_DST}
