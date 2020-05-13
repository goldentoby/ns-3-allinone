DIR_PREFIX = "./tcp_udp"
set title "AMQ comparison"
set xlabel "Time (Seconds)"
set ylabel "Queueing Length (numbers)"
set terminal png font " Times_New_Roman, 10 "
set output DIR_PREFIX."/queuelength.png"
set xtics 1 ,30 ,200
set size .85, 1
set key at 260,200
set key right 

plot \
DIR_PREFIX."/FifoQueueDisc/queueTraces/queue.plotme" using 1:2 with linespoints linewidth .2 title "FIFO", \
DIR_PREFIX."/CoDelQueueDisc/queueTraces/queue.plotme" using 1:2 with linespoints linewidth .2 title "CODEL", \
DIR_PREFIX."/CobaltQueueDisc/queueTraces/queue.plotme" using 1:2 with linespoints linewidth .2 title "COBALT", \
DIR_PREFIX."/FqCoDelQueueDisc/queueTraces/queue.plotme" using 1:2 with linespoints linewidth .2 title "FQCODEL", \
DIR_PREFIX."/PfifoFastQueueDisc/queueTraces/queue.plotme" using 1:2 with linespoints linewidth .2 title "PFIFO", \
DIR_PREFIX."/PieQueueDisc/queueTraces/queue.plotme" using 1:2 with linespoints linewidth .2 title "PIE", \
DIR_PREFIX."/RedQueueDisc/queueTraces/queue.plotme" using 1:2 with linespoints linewidth .2 title "RED", \
DIR_PREFIX."/TbfQueueDisc/queueTraces/queue.plotme" using 1:2 with linespoints linewidth .2 title "TBF" \
