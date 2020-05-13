DIR_PREFIX = "./tcp"
set title "AMQ comparison"
set xlabel "Time (Seconds)"
set ylabel "Congestion Window (numbers)"
set terminal png font " Times_New_Roman, 10 "
set output DIR_PREFIX."/congestionwnd.png"
set xtics 1 ,30 ,200
set key right 

plot \
DIR_PREFIX."/FifoQueueDisc/cwndTraces/S1-1.plotme" using 1:2 with linespoints linewidth .2 title "FIFO", \
DIR_PREFIX."/CoDelQueueDisc/cwndTraces/S1-1.plotme" using 1:2 with linespoints linewidth .2 title "CODEL", \
DIR_PREFIX."/CobaltQueueDisc/cwndTraces/S1-1.plotme" using 1:2 with linespoints linewidth .2 title "COBALT", \
DIR_PREFIX."/FqCoDelQueueDisc/cwndTraces/S1-1.plotme" using 1:2 with linespoints linewidth .2 title "FQCODEL", \
DIR_PREFIX."/PfifoFastQueueDisc/cwndTraces/S1-1.plotme" using 1:2 with linespoints linewidth .2 title "PFIFO", \
DIR_PREFIX."/PieQueueDisc/cwndTraces/S1-1.plotme" using 1:2 with linespoints linewidth .2 title "PIE", \
DIR_PREFIX."/RedQueueDisc/cwndTraces/S1-1.plotme" using 1:2 with linespoints linewidth .2 title "RED", \
DIR_PREFIX."/TbfQueueDisc/cwndTraces/S1-1.plotme" using 1:2 with linespoints linewidth .2 title "TBF" \
