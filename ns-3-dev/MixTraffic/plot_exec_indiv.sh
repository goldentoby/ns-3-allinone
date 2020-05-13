gnuplot -e "args='COBALT'; testcase='$1'" congestion_window_indiv.gp
gnuplot -e "args='FIFO'; testcase='$1'" congestion_window_indiv.gp
gnuplot -e "args='CODEL'; testcase='$1'" congestion_window_indiv.gp
gnuplot -e "args='FQCODEL'; testcase='$1'" congestion_window_indiv.gp
gnuplot -e "args='PFIFO'; testcase='$1'" congestion_window_indiv.gp
gnuplot -e "args='PIE'; testcase='$1'" congestion_window_indiv.gp
gnuplot -e "args='RED'; testcase='$1'" congestion_window_indiv.gp
gnuplot -e "args='TBF'; testcase='$1'" congestion_window_indiv.gp

gnuplot -e "args='COBALT'; testcase='$1'" queue_length_indiv.gp
gnuplot -e "args='FIFO'; testcase='$1'" queue_length_indiv.gp
gnuplot -e "args='CODEL'; testcase='$1'" queue_length_indiv.gp
gnuplot -e "args='FQCODEL'; testcase='$1'" queue_length_indiv.gp
gnuplot -e "args='PFIFO'; testcase='$1'" queue_length_indiv.gp
gnuplot -e "args='PIE'; testcase='$1'" queue_length_indiv.gp
gnuplot -e "args='RED'; testcase='$1'" queue_length_indiv.gp
gnuplot -e "args='TBF'; testcase='$1'" queue_length_indiv.gp
