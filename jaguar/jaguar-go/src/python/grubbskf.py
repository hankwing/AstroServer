import pandas as pd
import numpy as np
import time
import os, os.path, datetime

# start = time.clock()
df = pd.DataFrame()
lastfile = []
m = []
cnt = 0

# KalmanFilter
Q = 1e-5  # process variance
R = 0.05 ** 2  # estimate of measurement variance, change to see effect
xhat = []  # a posterity estimate of x
P = []  # a posterity error estimate
xhatminus = []  # a priority estimate of x
Pminus = []  # a priority error estimate
K = []  # gain or blending factor

# Kalman
datatotal = 0
dataavg = 0
# anomalySet = np.empty_like(dataSet)
# anomalySet[:] = np.nan
anomalyCount = []
time_stamp = 0  #
index = 0  #
anomalyCount = 0

folder = '/home/wamdm/jaguar/jaguar-go/data/'
# folder = "E:\\pythonfile\\lightCurveDemo\\tswithlstm\\readdata\\test\\data\\"
name = os.listdir(folder)
# names = name.sort()
for n in name:
    alldata_pre = pd.read_table(folder + n, index_col=False, header=None, delim_whitespace=True)
    data_pre = alldata_pre.iloc[:, 1]  #
    df = pd.concat([df, data_pre], axis=1)  #
print(df)

current_time = time.localtime()
tm_year = str(current_time.tm_year)
tm_mon = str(current_time.tm_mon)

if current_time.tm_mon < 10:
    tm_mon = '0' + tm_mon

tm_mday = str(current_time.tm_mday)

if current_time.tm_mday < 10:
    tm_mday = '0' + tm_mday

tm_hour = str(current_time.tm_hour)

if current_time.tm_hour < 10:
    tm_hour = '0' + tm_hour

tm_min = str(current_time.tm_min)

if current_time.tm_min < 10:
    tm_min = '0' + tm_min

tm_sec = str(current_time.tm_sec)

if current_time.tm_sec < 10:
    tm_sec = '0' + tm_sec

out_file_name = tm_year + '_' + tm_mon + '_' + tm_mday + '_' \
                + tm_hour + '_' + tm_min + '_' + tm_sec

while True:
    # print(1)
    today = str(datetime.date.today())
    # out = open('../data/result/' + out_file_name + '.txt', 'a')

    cnt += 1
    l = os.listdir(folder)
    if l == []:
        pass
    elif lastfile != l[-1]:
        lastfile = l[-1]
        time.sleep(1)
        alldata = pd.read_table(folder + lastfile, index_col=False, header=None, delim_whitespace=True)
        # print(alldata)
        data1 = alldata.iloc[:, 1]  #
        df = pd.concat([df, data1], axis=1)  #

        for n in range(50):
            d = df.iloc[n, -50:-1]  #

            mean = np.mean(d)
            current_value = df.iloc[n, -1]
            max = np.max(d)
            min = np.min(d)
            S = np.std(d)
            if current_value > max:
                residual = current_value - mean  #
                if residual > (mean - min):
                    G = residual / S
                    if G > 0.168:  # 2.768
                        m.append(n)
                        print(m[-1] + 1)
                        data_error = df.iloc[n, :]
                        print(data_error)
                        # KalmanFilter start
                        # for data in data_error:
                        # for data in data_error:
                        #     if time_stamp < 1:
                        #         xhat.append(data)  #
                        #         P.append(1.0)  #
                        #         xhatminus.append(0)  #
                        #         Pminus.append(0)
                        #         K.append(0)
                        #     else:
                        #         # time update
                        #         xhatminus.append(xhat[index - 1])  # X(k|k-1) = AX(k-1|k-1) + BU(k) + W(k),A=1,BU(k) = 0
                        #         Pminus.append(P[index - 1] + Q)  # P(k|k-1) = AP(k-1|k-1)A' + Q(k) ,A=1
                        #         # measurement update
                        #         K.append(Pminus[index] / (Pminus[index] + R))
                        #         xhat.append(xhatminus[index] + K[index] * (data - xhatminus[index]))
                        #         P.append((1 - K[index]) * Pminus[index])
                        #
                        #     #
                        #     datatotal += data
                        #     dataavg = datatotal / (index + 1)
                        #     errorRate = abs(xhat[index] - data) / data
                        #     if errorRate >= 0.05 and abs(data - dataavg) / data >= 0.05:
                        #
                        #         # anomalySet[index] = data
                        #         anomalyCount += 1
                        #     time_stamp += 1
                        #     index += 1
                        # KalmanFilter end
            m = list(set(m))
            # for i in m:
            #     dm = df.iloc[i, :]
            #     # print(dm)
            #     # dm.index = pd.date_range('2015-07-01', periods=cnt, freq='15s')
            #     plt.plot(df.iloc[i, :])
            #     plt.show()
        else:
            pass
        # out.close()  # end = time.clock()
        # print('read:%f s' % (end-start))
        # time.sleep(1)
