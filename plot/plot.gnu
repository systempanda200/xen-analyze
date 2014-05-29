set term png enhanced size 1024, 768 font 'Times-Roman'
set output 'd3_vcpus.png'
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
	'd3v0.txt' using 2:3 title 'd3v0' with p lw 2, \
	'd3v1.txt' using 2:3 title 'd3v1' with p lw 2
#plot \
#	'< paste d3v0.txt d3v1.txt' using 2:3 title 'd3v0' with p lw 2, \
#	'< paste d3v0.txt d3v1.txt' using 5:6 title 'd3v1' with p lw 2
