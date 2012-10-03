#!/bin/bash
args="$@"
count=0
counter=""
while [ "$1" != "" ]; do
    case $1 in
        -count )	count=1
			;;
    esac
    shift
done

if [ "$count" = "1" ]; then
    args=`echo $args | sed -e "s/-count / /g"`
    counter=`pin -t $QUADHOME/QUAD.so -count_only $args 2>&1 | grep "Counted Instructions:"`
    counter=`echo $counter | sed 's/^\([A-Za-z :]*\)\([0-9]*\)\([ M\+]*\)\([0-9]*\)/-m_ins \2 -ins \4/'`
fi

pin -t $QUADHOME/QUAD.so $counter $args > output 
