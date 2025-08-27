#!/usr/bin/env bash

source testing.bash
source util.bash

result=0

#########
# ffAdd #
#########
ffAdd 0x57 0x83 result
expect_equal_int 0xd4 "${result}"

#########
# xtime #
#########
xtime 0x57 result
expect_equal_int 0xae "${result}"
xtime 0xae result
expect_equal_int 0x47 "${result}"
xtime 0x47 result
expect_equal_int 0x8e "${result}"
xtime 0x8e result
expect_equal_int 0x07 "${result}"

##############
# ffMultiply #
##############
ffMultiply 0x57 0x13 result
expect_equal_int 0xfe "${result}"

####################
# extended_euclids #
####################
extended_euclids 3 256 result
expect_equal_int 171 $((result))

extended_euclids 0 256 result
expect_equal_int 0 $((result))

###########
# SubByte #
###########
SubByte 0x13 result
expect_equal_int 0x7d $((result))

SubByte 0xff result
expect_equal_int 0x16 $((result))

SubByte 0x00 result
expect_equal_int 0x63 $((result))

echo "All Tests Passed"
