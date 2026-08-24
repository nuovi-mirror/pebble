#!/bin/sh

# script to auto-link POSIX items

# common
ln -s ../common/cmpstr.h platform/use/cmpstr.h
ln -s ../common/copystr.h platform/use/copystr.h
ln -s ../common/findnewline.h platform/use/findnewline.h
ln -s ../common/getstrlen.h platform/use/getstrlen.h
ln -s ../common/skipspace.h platform/use/skipspace.h

# POSIX
ln -s ../posix/readfile.h platform/use/readfile.h
ln -s ../posix/print.h platform/use/print.h
