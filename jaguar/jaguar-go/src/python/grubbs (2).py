'''
@File    : 真是数据.py
@Author  : Jack Feng
@Time    : 2017/1/2 20:52
'''

import pandas as pd
import numpy as np
import time
import os, os.path, datetime
import fcntl

# start = time.clock()
df = pd.DataFrame()
lastfile = []
m = []
cnt = 0

# 先读取文件夹内已有文件
folder = "C:\\Users\\27467\\Documents\\pycharm\\Tsinghua\\data\\1114\\matrix\\"
name = os.listdir(folder)
# names = name.sort()
for n in name:
    file_handle = open(folder + n, 'r')  # 打开文件
    fcntl.flock(file_handle, fcntl.LOCK_EX | fcntl.LOCK_NB)  # 加锁

    alldata_pre = pd.read_table(file_handle, index_col=False, header=None, delim_whitespace=True)

    data_pre = alldata_pre.iloc[:, 1]
    df = pd.concat([df, data_pre], axis=1)  # 0为行拼接

    fcntl.flock(file_handle, fcntl.LOCK_UN) #解锁
    file_handle.close() #关闭文件
# print(df)

# 持续运行程序 读取新加入的最后一个文件
while True:
    today = str(datetime.date.today())
    out = open('C:\\Users\\27467\\Documents\\pycharm\\Tsinghua\\data\\result\\' + today + '.txt', 'a')

    cnt += 1
    l = os.listdir(folder)
    # l.sort(key=lambda fn: os.path.getmtime(base_dir + fn) if not os.path.isdir(base_dir + fn) else 0)
    # d = datetime.datetime.fromtimestamp(os.path.getmtime(base_dir+l[-1]))
    # print('最后改动的文件是'+l[-1]+"，时间："+d.strftime("%Y.%m.%d. %H:%M:%S"))
    # print(l[-1])
    if l == []:
        pass
    elif lastfile != l[-1]:
        lastfile = l[-1]
        time.sleep(1)
        file_handle = open(folder + lastfile, 'r')  # 打开文件
        alldata = pd.read_table(file_handle, index_col=False, header=None, delim_whitespace=True)
        fcntl.flock(file_handle, fcntl.LOCK_EX | fcntl.LOCK_NB)  # 加锁

        # print(alldata)
        data1 = alldata.iloc[:, 1]
        df = pd.concat([df, data1], axis=1)     #0为行拼接
        # print(df)
        for n in range(len(df)):
            d = df.iloc[n, -50:-1]

            #格拉布斯准则
            mean = np.mean(d)
            current_value = df.iloc[n, -1]
            max = np.max(d)
            min = np.min(d)
            S = np.std(d)
            if current_value > max:
                residual = current_value - mean         #残差
                if residual > (mean - min):
                    G = residual/S
                    if G > 2.956:       #α=0.05     n=50
                        m.append(n)
                        # print(m[-1]+1, time.strftime('%H:%M:%S'), lastfile)
                        # print(m[-1]+1, time.strftime('%H:%M:%S'), lastfile, file=out)
                        # print(m[-1] + 1, file=out)
                        print(alldata.iloc[n, 0])
                        # 格拉布斯准则 end

        m = list(set(m))
        # for i in m:
        #     dm = df.iloc[i, :]
        #     # print(dm)
        #     # dm.index = pd.date_range('2015-07-01', periods=cnt, freq='15s')
        #     plt.plot(df.iloc[i, :])
        #     plt.show()
    else:
        pass
    out.close()

    fcntl.flock(file_handle, fcntl.LOCK_UN)  # 解锁
    file_handle.close()  # 关闭文件
# end = time.clock()
    # print('read:%f s' % (end-start))
    # time.sleep(1)

