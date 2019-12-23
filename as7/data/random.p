#Gnuplot script for plotting data in file "random.dat"
set terminal png
set output "graph.png"

set title "Page replacement using random policy"
set key right center
set xlabel "frames in memory"
set ylabel "hit ratio"

set xrange [0:100]
set yrange [0:1]

plot "random.dat" u 1:2 w linespoints title "random"
plot "optimal.dat" u 1:2 w linespoints title "optimal"