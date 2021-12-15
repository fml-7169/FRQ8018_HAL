#!/bin/bash -
#===============================================================================
#
#          FILE: my.sh
#
#         USAGE: ./my.sh
#
#   DESCRIPTION: 
#
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (), 
#  ORGANIZATION: 
#       CREATED: Monday, April 12, 2021 05:09:36 HKT
#      REVISION:  ---
#===============================================================================

set -o nounset                                  # Treat unset variables as an error
./govee_hal_build.sh 1
cp ../../platform/hal/libs/libgovee_hal.a  ../../../ESD_FR801xH-Govee/platform/hal/libs/libgovee_hal.a
