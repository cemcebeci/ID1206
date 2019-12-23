set terminal png
set output "tlb.png"
set title "TLB benchmark, 4KiByte pages, 10 Gi Operations"
set key right center
set xlabel "number of pages"
set ylabel "time in s"
set logscale x 2
plot "tlb.dat" u 1:2 w linespoints title "page refs"
