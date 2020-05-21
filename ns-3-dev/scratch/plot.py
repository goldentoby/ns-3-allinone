import numpy as np 
from matplotlib import pyplot as plt 
 
x = np.arange(0,900) 
y =  []

with open('./traffic_h264.txt') as read_file:
    for line in read_file:
        y.append(int(line))

plt.title("H.264 Video Frame Size") 
plt.xlabel("Frame Index") 
plt.ylabel("Frame Size") 
plt.plot(x,np.array(y))
plt.show()