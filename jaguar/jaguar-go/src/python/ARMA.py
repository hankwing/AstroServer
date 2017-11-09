
import pandas as pd
from statsmodels.tsa.arima_model import ARMA
import matplotlib.pyplot as plt
import numpy as np
import time
import os, os.path, datetime
from statsmodels.tsa.stattools import adfuller as ADF
import re
import fcntl

# start = time.clock()
df = pd.DataFrame()
lastfile = []
m = []
p1 = []
p2 = []
p3 = []
p4 = []
p5 = []
cnt = 0

folder = "/home/wamdm/jaguar/jaguar-go/data/"
name = os.listdir(folder)
# names = name.sort()

for n in name:
    file_handle = open(folder + n, 'r') 
    fcntl.flock(file_handle, fcntl.LOCK_EX | fcntl.LOCK_NB)  
    out = open('/home/wamdm/jaguar/jaguar-go/result/' + n, 'a')
    alldata_pre = pd.read_table(file_handle, index_col=False, header=None, delim_whitespace=True)
    data_pre = alldata_pre.iloc[:, 1]
    df = pd.concat([df, data_pre], axis=1) 

    fcntl.flock(file_handle, fcntl.LOCK_UN) 
    file_handle.close() 
# print(df)

while True:
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
        out = open('/home/wamdm/jaguar/jaguar-go/result/' + lastfile, 'a')

        time.sleep(1)
        alldata = pd.read_table(folder + lastfile, index_col=False, header=None, delim_whitespace=True)
        # print(alldata)
        data1 = alldata.iloc[:, 1]
        df = pd.concat([df, data1], axis=1)   
        # print(df)
        for n in range(len(df)):
            d = df.iloc[n, -50:-1]

 
            mean = np.mean(d)
            current_value = df.iloc[n, -1]
            max = np.max(d)
            min = np.min(d)
            S = np.std(d)
            if current_value > max:
                residual = current_value - mean      
                if residual > (mean - min):
                    G = residual/S
                    if G > 2.956:       #α=0.05     n=50
                        # m.append(n)
                        # print(m[-1]+1, time.strftime('%H:%M:%S'), lastfile)
                        # print(m[-1]+1, time.strftime('%H:%M:%S'), lastfile, file=out)
                        # print(alldata.iloc[n, 0])
                        # print(alldata.iloc[n, 0], file=out)


          
                        data_ARMA = df.iloc[n, :]
                        per = 50                             
                        a = []
                        for i in range(per, len(data_ARMA)):
                            data_ARMA_per = df.iloc[n, i-per:i]
                            A = ADF(data_ARMA_per)              
                            res = re.findall(".*'5%': ([-+]?[0-9]*\.?[0-9]*)", str(A[4]), flags=0)
                            d = 0
                            p = 1
                            q = 0

                            if A[0] > float(res[0]):       
                                d += 1
                                diff1 = data_ARMA_per.diff(d)
                                A = ADF(diff1.iloc[d:, ])

                                model1 = ARMA(diff1, (p, q)).fit()
                                dp_value = model1.predict(per, per + 4)   
                                p_1 = current_value + dp_value[0]       
                                p_2 = p_1 + dp_value[1]
                                p_3 = p_2 + dp_value[2]
                                p_4 = p_3 + dp_value[3]
                                p_5 = p_4 + dp_value[4]

                                p1.append(p_1)
                                p2.append(p_2)
                                p3.append(p_3)
                                p4.append(p_4)
                                p5.append(p_5)
                                if i > per +5:    
                                    err1 = abs(((p1[-1] + p2[-2] + p3[-3] + p4[-4] + p5[-5]) / 5) - current_value)

                            else:           
                                data_ARMA_per.index = pd.date_range('13/10/2016', periods=per, freq='15S')
                                model1 = ARMA(data_ARMA_per, (p, q)).fit()
                                dp_value = model1.predict(per, per + 4) 
                                p_1 = dp_value[0]
                                p_2 = dp_value[1]
                                p_3 = dp_value[2]
                                p_4 = dp_value[3]
                                p_5 = dp_value[4]

                                p1.append(p_1)
                                p2.append(p_2)
                                p3.append(p_3)
                                p4.append(p_4)
                                p5.append(p_5)
                                if i > per +5:
                                    err1 = abs(((p1[-1] + p2[-2] + p3[-3] + p4[-4] + p5[-5]) / 5) - current_value)
                            if i > per+5:
                                a.append(err1)
                        # current_error = a[-1]
                        errlimit = np.mean(a)
                        for r in range(5):             
                            if a[-r] > errlimit:
                                # print('warning:', i + 2, err1, file=out)
                                print(alldata.iloc[n, 0])
                                print(alldata.iloc[n, 0], file=out)
                                break
                  

    else:
        pass
    out.close()

# end = time.clock()
    # print('read:%f s' % (end-start))
    # time.sleep(1)


