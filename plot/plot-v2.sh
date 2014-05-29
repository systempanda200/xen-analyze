#!/bin/bash
#echo "dom is $1, vcpu is $2, $3"
domID=$1
vcpu0=$2
vcpu1=$3
read -p "dom is $1, vcpu is $2, $3;\n press any key to continue" -t 5
#xenalyze --scatterplot-pcpu /tmp/trace.bin | grep ^${domID}v${vcpu0} > d${domID}v${vcpu0}.txt
#xenalyze --scatterplot-pcpu /tmp/trace.bin | grep ^${domID}v${vcpu1} > d${domID}v${vcpu1}.txt

gnuplot << EOF
    set term png enhanced size 1024, 768 font 'Times-Roman'
    set output 'd${domID}_vcpus.png'
    set title 'vCPU on pCPU execution'
    set grid
    set key outside bottom center horizontal
    set xrange [0:]
    set yrange [0:16]
    set xtics 100
    set ytics 16
    set xlabel 'Time'
    set ylabel 'pCPU'
    plot \
        'd${domID}v${vcpu0}.txt' using 2:3 title 'd${domID}v${vcpu0}' with p lw 2, \
        'd${domID}v${vcpu1}.txt' using 2:3 title 'd${domID}v${vcpu1}' with p lw 2
EOF
open d${domID}_vcpus.png
