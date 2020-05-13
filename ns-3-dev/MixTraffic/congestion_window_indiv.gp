PARAM = args
DIR_PREFIX = "./".testcase
DIR = ""
set title "AMQ-".PARAM
set xlabel "Time (Seconds)"
set ylabel "Congestion Window (Numbers)"
set terminal png font " Times_New_Roman, 10 "
set output DIR_PREFIX."/".PARAM."_cwnd_indiv.png"
set xtics 1 ,30 ,200
# set key at 260,200
set key right 

print PARAM
if (PARAM eq "COBALT") DIR = DIR_PREFIX."/CobaltQueueDisc/cwndTraces/S1-1.plotme" 
if (PARAM eq "FIFO") DIR = DIR_PREFIX."/FifoQueueDisc/cwndTraces/S1-1.plotme"
if (PARAM eq "CODEL") DIR = DIR_PREFIX."/CoDelQueueDisc/cwndTraces/S1-1.plotme"
if (PARAM eq "FQCODEL") DIR = DIR_PREFIX."/FqCoDelQueueDisc/cwndTraces/S1-1.plotme"
if (PARAM eq "PFIFO") DIR = DIR_PREFIX."/PfifoFastQueueDisc/cwndTraces/S1-1.plotme"
if (PARAM eq "PIE") DIR = DIR_PREFIX."/PieQueueDisc/cwndTraces/S1-1.plotme"
if (PARAM eq "RED") DIR = DIR_PREFIX."/RedQueueDisc/cwndTraces/S1-1.plotme"
if (PARAM eq "TBF") DIR = DIR_PREFIX."/TbfQueueDisc/cwndTraces/S1-1.plotme"


print testcase
if (testcase eq "tcp_tcp") {
    DIR_PLOT2= system("echo ".DIR." | sed s=S1-1.plotme=S1-2.plotme=g");
    print DIR_PLOT2
    plot \
    DIR using 1:2 with linespoints linewidth .2 title "cwnd1", \
    DIR_PLOT2 using 1:2 with linespoints linewidth .2 title "cwnd2", \
}else{
    plot DIR using 1:2 with linespoints linewidth .2 title PARAM
}