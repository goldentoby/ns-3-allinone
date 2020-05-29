# # Initial frame size distribution 
# import numpy as np 
# from matplotlib import pyplot as plt 
 
# x = np.arange(0,900) 
# y =  []
# addi = 0
# with open('./traffic_h264.txt') as read_file:
#     for line in read_file:
#         y.append(int(line))
#         if int(line) > 65000: addi+=1

# print(addi)
# plt.title("H.264 Video Frame Size") 
# plt.xlabel("Frame Index") 
# plt.ylabel("Frame Size") 
# plt.xlim((0, 902))
# xmarks=[i for i in range(0,901,100)]
# plt.xticks(xmarks)
# plt.bar(x,np.array(y))
# plt.savefig("../../../../../../../Desktop/plot/frame_over_time.png")
# # plt.show()

# # ==================================================================================

# CoDelQueueDisc
# CobaltQueueDisc
# FifoQueueDisc
# FqCoDelQueueDisc
# PfifoFastQueueDisc
# PieQueueDisc
# RedQueueDisc
queue_types = [
    'CoDelQueueDisc',
    'CobaltQueueDisc',
    'FifoQueueDisc',
    'FqCoDelQueueDisc',
    'PfifoFastQueueDisc',
    'PieQueueDisc',
    'RedQueueDisc',
]
# queue_type = 'CoDelQueueDisc'

for queue_type in queue_types:

    # Packets sent, received, and lost(for each packet)
    import numpy as np 
    from matplotlib import pyplot as plt 
    
    x = []; x1 = []; x2 = []
    y =  []; y1 = []; y2 = []
    sender_dict = {}; receiver_dict = {} 

    with open('../{}_sender_packet_stream.csv'.format(queue_type)) as read_file:
        for line in read_file:
            token = line.split(',')
            x.append(int(token[0]))
            y.append(int(token[1]))
            sender_dict[token[0]] = token[1]
            

    with open('../{}_receiver_packet_stream.csv'.format(queue_type)) as read_file:
        for line in read_file:
            token = line.split(',')
            x1.append(int(token[0]))
            y1.append(int(token[1]))
            receiver_dict[token[0]] = token[1]


    for k, v in sender_dict.items():
        if k not in receiver_dict:
            x2.append(int(k))
            y2.append(int(v))
            
    plt.figure(figsize=(18,13))
    plt.subplot(3,1,1)
    plt.bar(x,np.array(y))
    plt.xlim((0, 1025))
    plt.title("Sender Side - {}".format(queue_type),fontsize = 15)
    plt.ylabel("Packet Size",fontsize = 15)
    plt.xticks(fontsize= 12)
    plt.yticks(fontsize= 12)

    plt.subplot(3,1,2)
    plt.bar(x1,np.array(y1))
    plt.xlim((0, 1025))

    plt.title("Receiver Side - {}".format(queue_type),fontsize = 15) 
    plt.ylabel("Packet Size",fontsize = 15) 
    plt.xticks(fontsize= 12)
    plt.yticks(fontsize= 12)

    plt.subplot(3,1,3)
    plt.bar(x2,np.array(y2))
    plt.xlim((0, 1025))
    plt.title("Packet Loss - {}".format(queue_type),fontsize = 15)
    plt.ylabel("Packet Size",fontsize = 15)
    plt.xlabel("Packet Index",fontsize = 15)
    plt.xticks(fontsize= 12)
    plt.yticks(fontsize= 12)

    plt.savefig("../../../../../../../Desktop/plot/fig1a/{}.png".format(queue_type))

    # plt.show()

    # ==================================================================================

    # Packets sent, received, and lost(for each second)
    import numpy as np 
    import statistics
    from matplotlib import pyplot as plt 
    
    x = [i for i in range(28)]
    y =  [0 for _ in range(28)]

    x1 = [i for i in range(28)]
    y1 =  [0 for _ in range(28)]

    x2 = [i for i in range(28)]
    y2 =  [0 for _ in range(28)]

    time = 1
    with open('./traffic_h264.txt') as read_file:
        for idx, element in enumerate(read_file):
            if idx*0.03 <= time:
                y[time] += 1
                if int(element) > 65000: y[time] += 1
            else:
                time += 1
                y[time] += 1
                if int(element) > 65000: y[time] += 1

    tmp_x = []
    with open('../{}.csv'.format(queue_type)) as read_file:
        for idx, line in enumerate(read_file):
            token = line.split(',')

            tmp_x.append(float(token[0]))

        for idx in range(28):
            for i, time in enumerate(tmp_x):
                if idx-1< time < idx:
                    y1[idx] +=1

    for i in range(28):
        y2[i] = y[i] - y1[i]


    xmarks=[i for i in range(0,28,1)]
    ymarks=[i for i in range(0,49,4)]

    plt.figure(figsize=(18,13))
    plt.subplot(3,1,1)
    plt.xlim((0, 28))
    plt.ylim((0, 48))
    plt.title("Sender Side - {}".format(queue_type),fontsize = 15)
    plt.ylabel("packet number",fontsize = 15) 
    plt.xticks(fontsize= 12)
    plt.yticks(fontsize= 12)
    plt.xticks(xmarks)
    plt.yticks(ymarks)
    plt.bar(x,np.array(y))

    plt.subplot(3,1,2)
    plt.xlim((0, 28))
    plt.ylim((0, 48))
    
    plt.title("Receiver Side - {}".format(queue_type),fontsize = 15)
    plt.ylabel("packet number",fontsize = 15) 
    plt.xticks(fontsize= 12)
    plt.yticks(fontsize= 12)
    plt.xticks(xmarks)
    plt.yticks(ymarks)
    plt.bar(x1,np.array(y1))


    plt.subplot(3,1,3)
    plt.xlim((0, 28))
    plt.ylim((0, 48))
    plt.title("Packet Loss - {}".format(queue_type),fontsize = 15)
    plt.xlabel("time (second)",fontsize= 15) 
    plt.ylabel("packet number",fontsize= 15)
    plt.xticks(fontsize= 12)
    plt.yticks(fontsize= 12)
    plt.xticks(xmarks)
    plt.yticks(ymarks)
    plt.bar(x2,np.array(y2))
    plt.savefig("../../../../../../../Desktop/plot/fig1b/{}.png".format(queue_type))

    # plt.show()

    # ==================================================================================

    # Latency over time
    import numpy as np 
    import statistics
    from matplotlib import pyplot as plt 
    
    x = []
    y =  []
    x1 = [i for i in range(28)]
    y1 =  [0 for _ in range(28)]
    xa = []
    xb = []
    thres = 0

    with open('../{}.csv'.format(queue_type)) as read_file:
        for idx, line in enumerate(read_file):
            token = line.split(',')
            x.append(float(token[0]))
            y.append(float(token[1]))
            if float(token[0]) <= 27/2: xa.append(float(token[1]))
            else: xb.append(float(token[1]))

    for idx in range(28):
        for i, time in enumerate(x):
            if idx-1< time < idx:
                y1[idx] +=1

    print(statistics.mean(xa), statistics.mean(xb))

    plt.figure(figsize=(18,7))
    plt.subplot(2,1,1)
    plt.title("Latency over time - {}".format(queue_type),fontsize = 15) 
    # plt.xlabel("time (second)") 
    plt.ylabel("Latency (milli second)", fontsize = 15) 
    xmarks=[i for i in range(0,28,1)]
    plt.axvline(27/2, 0, 350, color='red')
    plt.xticks(xmarks)
    plt.bar(x,np.array(y))

    plt.subplot(2,1,2)
    plt.title("Packet sent over time - {}".format(queue_type),fontsize = 15)
    plt.xlabel("time (second)",fontsize= 15) 
    plt.ylabel("packet number",fontsize= 15) 
    plt.xticks(fontsize= 12)
    plt.yticks(fontsize= 12)
    xmarks=[i for i in range(0,28,1)]
    plt.xticks(xmarks)
    plt.bar(x1,np.array(y1))
    plt.savefig("../../../../../../../Desktop/plot/fig2/{}.png".format(queue_type))
    # plt.show()

    # ==================================================================================

    # queue occupancy over time
    import numpy as np 
    from matplotlib import pyplot as plt 
    
    x = []
    y =  []

    with open('../Multimedia_packet/{}/queueTraces/queue.plotme'.format(queue_type)) as read_file:
        for line in read_file:
            token = line.split(' ')
            x.append(float(token[0]))
            y.append(float(token[1]))

    plt.figure(figsize=(18,4))
    plt.title("Queue occupancy over time - {}".format(queue_type),fontsize = 15) 
    plt.xlabel("Time (seconds)",fontsize= 15) 
    plt.ylabel("Queue Length",fontsize= 15) 
    plt.xticks(fontsize= 12)
    plt.yticks(fontsize= 12)
    plt.xlim((0, 27))
    plt.ylim((0, 200))
    xmarks=[i for i in range(0,27,5)]
    plt.xticks(xmarks)
    plt.bar(x,np.array(y))
    plt.savefig("../../../../../../../Desktop/plot/fig3/{}.png".format(queue_type))
    # plt.show()