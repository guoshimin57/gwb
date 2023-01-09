#!/bin/sh

# *************************************************************************
#     mk_logo：生成gwb的圖標文件。
#     版權 (C) 2023 gsm <406643764@qq.com>
#     本程序為自由軟件：你可以依據自由軟件基金會所發布的第三版或更高版本的
# GNU通用公共許可證重新發布、修改本程序。
#     雖然基于使用目的而發布本程序，但不負任何擔保責任，亦不包含適銷性或特
# 定目標之適用性的暗示性擔保。詳見GNU通用公共許可證。
#     你應該已經收到一份附隨此程序的GNU通用公共許可證副本。否則，請參閱
# <http://www.gnu.org/licenses/>。
# *************************************************************************

# logo尺寸，即正方形的邊長，單位爲像素
a=256
ratio=0.9

#以下變量不應修改
offset=$(echo "$a*(1-$ratio)" | bc -l)
r=$(echo "$a/2-$offset" | bc -l)
cx=$(echo "$a/2-1" | bc -l)
cy=$cx
pi=$(echo "4*a(1)" | bc -l)
sin60=$(echo "s($pi/3)" | bc -l)
w=$(echo "$r*$sin60" | bc -l)
h=$(echo "$r/2" | bc -l)
lx=$(echo "$cx-$w" | bc -l)
rx=$(echo "$cx+$w" | bc -l)
ty=$(echo "$offset-1" | bc -l)
by=$(echo "$cy+$h" | bc -l)

# 以下變量應根據字體調整，特別是fontsize前的縮放系數，用以取得字符實際大小
fontname="FreeMono"
fontsize=$(echo "$r*0.8" | bc -l)
Gx=$(echo "$cx-$r/2-0.5*$fontsize/2" | bc -l)
Gy=$cy
Wx=$(echo "$cx+$r/2-0.6*$fontsize/2" | bc -l)
Wy=$cy
Bx=$(echo "$cx-0.5*$fontsize/2" | bc -l)
By=$(echo "$cy+$r/2+0.6*$fontsize/2" | bc -l)

convert -size ${a}x${a} xc:none -stroke black -strokewidth 3 \
    -fill red -draw "path 'M $cx,$cy L $cx,$ty A $r,$r 0 0,1 $rx,$by Z'" \
    -fill blue -draw "path 'M $cx,$cy L $rx,$by A $r,$r 0 0,1 $lx,$by Z'" \
    -fill green -draw "path 'M $cx,$cy L $lx,$by A $r,$r 0 0,1 $cx,$ty Z'" \
    -fill white -font $fontname -pointsize $fontsize -strokewidth 2 \
    -draw "text $Bx,$By 'B' text $Wx,$Wy 'W' text $Gx,$Gy 'G'" \
    gwb.png
