gnuplot -e "args='CODEL'" queue_length_indiv.gp
gnuplot -e "args='COBALT'" queue_length_indiv.gp
gnuplot -e "args='FIFO'" queue_length_indiv.gp

gnuplot -e "args='FQCODEL'" queue_length_indiv.gp
gnuplot -e "args='PFIFO'" queue_length_indiv.gp
gnuplot -e "args='PIE'" queue_length_indiv.gp
gnuplot -e "args='RED'" queue_length_indiv.gp
