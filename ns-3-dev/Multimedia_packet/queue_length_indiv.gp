PARAM = args
DIR_PREFIX = "."
DIR = ""
set title "AQM-".PARAM
set xlabel "Time (Seconds)"
set ylabel "Queueing Length (Numbers)"
set terminal png font " Times_New_Roman, 10 "
set output DIR_PREFIX."/".PARAM."_qlen_indiv.png"
set xtics 1 ,5 ,40
set key at 260,200
set key right 

print PARAM
if (PARAM eq "COBALT") DIR = DIR_PREFIX."/CobaltQueueDisc/queueTraces/queue.plotme" 
if (PARAM eq "FIFO") DIR = DIR_PREFIX."/FifoQueueDisc/queueTraces/queue.plotme"
if (PARAM eq "CODEL") DIR = DIR_PREFIX."/CoDelQueueDisc/queueTraces/queue.plotme"
if (PARAM eq "FQCODEL") DIR = DIR_PREFIX."/FqCoDelQueueDisc/queueTraces/queue.plotme"
if (PARAM eq "PFIFO") DIR = DIR_PREFIX."/PfifoFastQueueDisc/queueTraces/queue.plotme"
if (PARAM eq "PIE") DIR = DIR_PREFIX."/PieQueueDisc/queueTraces/queue.plotme"
if (PARAM eq "RED") DIR = DIR_PREFIX."/RedQueueDisc/queueTraces/queue.plotme"
if (PARAM eq "TBF") DIR = DIR_PREFIX."/TbfQueueDisc/queueTraces/queue.plotme"

plot DIR using 1:2 with linespoints linewidth .2 title PARAM
