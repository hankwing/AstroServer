package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"math"
	"net/http"
	"os"
	//"os/signal"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"log"
	//"time"
)

// the struct of a star object
type Star struct {
	XPix          float64
	YPix          float64
	Ra            float64
	Dec           float64
	Zone          int64
	StarId        int64
	Mag           float64
	Timestamp     float64
	Ellipticity   float64
	CcdNum        int64
}

// Result ... structure holds the result of anomaly detection
type Result struct {
	StarId        int
	Anomalous     bool
	MaxScore      float64
	AnomalyName   string
    AlgoNameList  []string
	AnomalousList []bool
}

type ResultArray []Result

type ajaxStruct struct {
	sync.Mutex
	FirstStarId         int
	SecondStarId        int
	FirstAnomalyName    string
	SecondAnomalyName   string
	AlgoNameList        []string
	FirstAnomalousList  []bool
	SecondAnomalousList []bool
	TimeList            []string
	FirstList           []float64
	SecondList          []float64
	FirstFlagList       []bool
	SecondFlagList      []bool
}

func (a ResultArray) Len() int           { return len(a) }
func (a ResultArray) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a ResultArray) Less(i, j int) bool { return a[i].MaxScore > a[j].MaxScore }

const (
	// NUMOFSTARS ... number of stars
	NUMOFSTARS = 1000000

	// HISTORYWIDTH ... length of long window
	HISTORYWIDTH = 2000

    // the length of three different kind of windows
    LONGWIDTH int = 45
	MIDWIDTH int = 15
	SHORTWIDTH int = 5

	// ENERGYTHRESHOLD ... threshold for triggering alert
	ENERGYTHRESHOLD = 0.992

	// TEMPLATETHRESHOLD ... threshold for template matching
	TEMPLATETHRESHOLD = 0.5
)

var (
	useTemplate                bool
	useDebugCache              bool
	noCache                    bool
	noFileOutput               bool
	indexPage                  string
	monitorPage                string

	fifoName                   string

	rawDataChannel             chan string

	dataChannel                [NUMOFSTARS + 1]chan Star
	fileNameChannel            chan string
	lineCountChannel           chan int
	resultChannel              chan Result

	templateList               [][]float64
	resultList                 []Result
	resultListLock             sync.Mutex

	historyWindow              [NUMOFSTARS + 1][]Star
	historyWindowLock          [NUMOFSTARS + 1]sync.Mutex

	longFlagWindow             [NUMOFSTARS + 1][]bool
	longFlagWindowLock         [NUMOFSTARS + 1]sync.Mutex

    longWindow                 [NUMOFSTARS + 1][]Star
	longWindowLock             [NUMOFSTARS + 1]sync.Mutex
	
	midWindow                  [NUMOFSTARS + 1][]Star
	midWindowLock              [NUMOFSTARS + 1]sync.Mutex
	
	shortWindow                [NUMOFSTARS + 1][]Star
	shortWindowLock            [NUMOFSTARS + 1]sync.Mutex

	ajaxResult                 ajaxStruct

	masterWaitGroup            sync.WaitGroup
	workerWaitGroup            sync.WaitGroup
)

//********************* utilities ***********************

func min(a, b int) int {
    if a < b {
        return a
    }
    return b
}

func minf(a, b float64) float64 {
    if a < b {
        return a
    }
    return b
}

func max(a, b int) int {
    if a > b {
        return a
    }
    return b
}

func maxf(a, b float64) float64 {
    if a > b {
        return a
    }
    return b
}

//********************* preprocessing ***********************

func loadRecentData() {
	log.Println("导入数据中......")
 
    var cacheDir string
	if useDebugCache {
        cacheDir = "../cache/debug.cache"
	} else {
		cacheDir = "../cache/cache.cache"
	}
	dataFile, err := os.OpenFile(cacheDir, os.O_RDONLY, 0777)
	if err != nil {
		panic(err)
	}
	
	recentDataReader := bufio.NewReader(dataFile)
	num := 0
	for {
		eachLine, err := recentDataReader.ReadString('\n')
		if err == io.EOF {
			log.Println("导入数据完毕")
			break
		}
		data_string_list := strings.Split(eachLine, " ")
		num++
		historyWindow[num] = make([]Star, 0)
		//no need to lock here since we only execute this code once when we run the whole system
		for _, elem := range data_string_list {
			value, _ := strconv.ParseFloat(elem, 64)
			var s Star
			s.Mag = value
			historyWindow[num] = append(historyWindow[num], s)
		}

		longWindow[num] = make([]Star, 0)

		midWindow[num] = make([]Star, 0)

		shortWindow[num] = make([]Star, 0)

		if num % 5000 == 0 || num == NUMOFSTARS {
			log.Printf("已导入行数：%d\n", num)
		}
	}

	
}

func saveRecentData() {
	log.Println("导出数据中......")
    
	var cacheDir string
	if useDebugCache {
        cacheDir = "../cache/debug.cache"
	} else {
		cacheDir = "../cache/cache.cache"
	}
	outRecentData, err := os.OpenFile(cacheDir, syscall.O_WRONLY | syscall.O_CREAT | syscall.O_TRUNC, 0666)
	if err != nil {
		panic(err)
	}

	for i := 1; i <= NUMOFSTARS; i++ {	
		historyWindowLock[i].Lock()
		for index, value := range historyWindow[i] {
			if index != 0 {
				outRecentData.WriteString(" ")
			}
			outRecentData.WriteString(strconv.FormatFloat(value.Mag, 'f', 5, 64))
		}
		historyWindowLock[i].Unlock()
		outRecentData.WriteString("\n")
		if i % 5000 == 0 || i == NUMOFSTARS {
			log.Printf("已导出行数：%d\n", i)
		}
	}

	outRecentData.Close()
	log.Println("导出数据完毕")
}

func genCosineTemp() {
	fsw := float64(SHORTWIDTH)
	for amp := 1; amp <= 10; amp++ {
		var template []float64
		for i := 0; i < SHORTWIDTH / 2; i++ {
			template = append(template, 0.0)
		}
		for i := SHORTWIDTH / 2; i < SHORTWIDTH; i++ {
			fi := float64(i)
			template = append(template, float64(amp) * math.Sin(math.Pi / fsw * (fi - fsw / 2.0)))
		}
		templateList = append(templateList, template)
	}

	for amp := 1; amp <= 10; amp++ {
		var template []float64
		for i := 0; i < SHORTWIDTH / 2; i++ {
			template = append(template, 0.0)
		}
		for i := SHORTWIDTH / 2; i < SHORTWIDTH; i++ {
			fi := float64(i)
			template = append(template, -float64(amp) * math.Sin(math.Pi / fsw * (fi - fsw / 2.0)))
		}
		templateList = append(templateList, template)
	}
}

func genLinearTemp() {
	fsw := float64(SHORTWIDTH)
	for amp := 1; amp <= 10; amp++ {
		var template []float64
		for i := 0; i < SHORTWIDTH / 2; i++ {
			template = append(template, 0.0)
		}
		for i := SHORTWIDTH / 2; i < SHORTWIDTH; i++ {
			fi := float64(i)
			template = append(template, float64(amp) * 0.1 * (fi - fsw / 2.0))
		}
		templateList = append(templateList, template)
	}

	for amp := 1; amp <= 10; amp++ {
		var template []float64
		for i := 0; i < SHORTWIDTH / 2; i++ {
			template = append(template, 0.0)
		}
		for i := SHORTWIDTH / 2; i < SHORTWIDTH; i++ {
			fi := float64(i)
			template = append(template, -float64(amp) * 0.1 * (fi - fsw / 2.0))
		}
		templateList = append(templateList, template)
	}
}

func genTemp() {
	genCosineTemp()
	genLinearTemp()
}

func loadTemp() {
	log.Println("导入模板数目：0")
}

//********************* anomaly detection ***********************

func tailProb(x float64) float64 {
	return 0.5 + 0.5 * math.Erf(x / math.Sqrt(2))
}

func createChannels() {
	rawDataChannel = make(chan string, 5)
	fileNameChannel = make(chan string, 5)
	lineCountChannel = make(chan int, 5)
	resultChannel = make(chan Result, NUMOFSTARS)
	for index := range dataChannel {
		dataChannel[index] = make(chan Star, 5)
	}
}

func loaderAndCleaner() {
	//打开管道
	pipe, err := os.OpenFile("../pipe/namedpipe", os.O_RDONLY, 0666)
	if err != nil {
		panic(err)
	}
	log.Println("已打开管道")
	defer pipe.Close()

	pipeReader := bufio.NewReader(pipe)
	for {
		rawData, err := pipeReader.ReadString('\n')

		if err == io.EOF { // EOF，等久一点
			log.Println("写端已关闭管道")
			close(rawDataChannel)
			break
		} else {
			rawDataChannel <- rawData
		}
	}

	log.Println("开始清理所有工作线程......")
	masterWaitGroup.Done()
	log.Println("loader已退出")
}

func dispatcher() {
	for {
		for {
			startFlag, active := <-rawDataChannel
			if !active { //捕获Ctrl+C信号，关闭管道
				close(fileNameChannel)
				close(lineCountChannel)
				for index := range dataChannel {
					close(dataChannel[index])
				}
				goto DONE
			}
			// 如果读到上一次的残留数据，直接忽略
			if startFlag == "start\n" {
				break
			}
		}

		//parse time
		fileName, active := <-rawDataChannel
		if !active { //捕获Ctrl+C信号，关闭管道
			close(fileNameChannel)
			close(lineCountChannel)
			for index := range dataChannel {
				close(dataChannel[index])
			}
			goto DONE
		}

		strippedFileName := fileName[:len(fileName)-1] // 去掉换行符
		fileNameChannel <- strippedFileName

		lineCount := 0
		for {
			line, active := <-rawDataChannel
			if !active { //捕获Ctrl+C信号，关闭管道
				close(fileNameChannel)
				close(lineCountChannel)
				for index := range dataChannel {
					close(dataChannel[index])
				}
				goto DONE
			}

			if line == "end\n" {
				lineCountChannel <- lineCount
				break
			}

			splitLine := strings.Fields(line)
			var s Star
			s.XPix, _ = strconv.ParseFloat(splitLine[0], 64)
			s.YPix, _ = strconv.ParseFloat(splitLine[1], 64)
			s.Ra, _ = strconv.ParseFloat(splitLine[2], 64)
			s.Dec, _ = strconv.ParseFloat(splitLine[3], 64)
			s.Zone, _ = strconv.ParseInt(splitLine[4], 10, 64)
			s.StarId, _ = strconv.ParseInt(splitLine[5], 10, 64)
			s.Mag, _ = strconv.ParseFloat(splitLine[6], 64)
			s.Timestamp, _ = strconv.ParseFloat(splitLine[7], 64)
			s.Ellipticity, _ = strconv.ParseFloat(splitLine[8], 64)
			s.CcdNum, _ = strconv.ParseInt(splitLine[9], 10, 64)

			if s.StarId >= 1 && s.StarId <= NUMOFSTARS {
				lineCount++
				dataChannel[s.StarId] <- s
			}
		}
	}
DONE:
	masterWaitGroup.Done()
	log.Println("dispatcher已退出")
}

func worker(starId int) {
	for {
		var curr Result

        curr.StarId = starId
		curr.MaxScore = 0
		curr.Anomalous = false

		s, active := <-dataChannel[starId]
		if !active { //捕获Ctrl+C信号，关闭管道
			goto DONE
		}

		historyWindowLock[starId].Lock()
		shortWindowLock[starId].Lock()
		midWindowLock[starId].Lock()
        longWindowLock[starId].Lock()

		historyWindow[starId] = append(historyWindow[starId], s)
		if len(historyWindow[starId]) > HISTORYWIDTH {
			historyWindow[starId] = historyWindow[starId][len(historyWindow[starId]) - HISTORYWIDTH:]
		}

		shortWindow[starId] = append(shortWindow[starId], s)
		if len(shortWindow[starId]) > SHORTWIDTH {
			shortWindow[starId] = shortWindow[starId][len(shortWindow[starId]) - SHORTWIDTH:]
		}

        midWindow[starId] = append(midWindow[starId], s)
		if len(midWindow[starId]) > MIDWIDTH {
			midWindow[starId] = midWindow[starId][len(midWindow[starId]) - MIDWIDTH:]
		}

		longWindow[starId] = append(longWindow[starId], s)
		if len(longWindow[starId]) > LONGWIDTH {
			longWindow[starId] = longWindow[starId][len(longWindow[starId]) - LONGWIDTH:]
		}

		longWindowLock[starId].Unlock()
		midWindowLock[starId].Unlock()
		shortWindowLock[starId].Unlock()
		historyWindowLock[starId].Unlock()

        //NFD detection families
		if len(historyWindow[starId]) == HISTORYWIDTH {

			historyMean := 0.0
			for _, s := range historyWindow[starId] {
				historyMean = historyMean + s.Mag
			}
			historyMean = historyMean / float64(len(historyWindow[starId]))

			historyStd := 0.0
			for _, s := range historyWindow[starId] {
				historyStd = historyStd + (s.Mag - historyMean) * (s.Mag - historyMean)
			}
			historyStd = math.Sqrt(historyStd / float64(len(historyWindow[starId])))

            if len(shortWindow[starId]) == SHORTWIDTH {

				//**************************** NFD_5 ****************************

				curr.AlgoNameList = append(curr.AlgoNameList, "NFD_5")

				shortMean := 0.0
				for _, s := range shortWindow[starId] {
					shortMean = shortMean + s.Mag
				}
				shortMean = shortMean / float64(len(shortWindow[starId]))
				shortNor := (shortMean - historyMean) / historyStd
				shortPrimaryScore := tailProb(shortNor)
				if shortPrimaryScore <= 0.5 {
					shortPrimaryScore = 1 - shortPrimaryScore
				}

				shortEnergy := shortPrimaryScore
				if shortEnergy > ENERGYTHRESHOLD {
					curr.AnomalousList = append(curr.AnomalousList, true)
					curr.Anomalous = true
				} else {
					curr.AnomalousList = append(curr.AnomalousList, false)
				}

				curr.MaxScore = maxf(curr.MaxScore, shortEnergy)

				//**************************** DIFF_5 ****************************
            	
				curr.AlgoNameList = append(curr.AlgoNameList, "DIFF_5")

				up_num := 0 
				for index := range shortWindow[starId] {
					if index > 0 {
						diff := shortWindow[starId][index].Mag - shortWindow[starId][index - 1].Mag
						if diff < 0 {
							up_num++
						}
					}
				}

				if up_num * 5 >= len(shortWindow[starId]) * 4 {
					curr.AnomalousList = append(curr.AnomalousList, true)
					curr.Anomalous = true
				} else {
					curr.AnomalousList = append(curr.AnomalousList, false)
				}

				curr.MaxScore = maxf(curr.MaxScore, float64(up_num) / float64(len(shortWindow[starId])))			
			}
			
            if len(midWindow[starId]) == MIDWIDTH {

				//**************************** NFD_15 ****************************

            	curr.AlgoNameList = append(curr.AlgoNameList, "NFD_15")

				midMean := 0.0
				for _, s := range midWindow[starId] {
					midMean = midMean + s.Mag
				}
				midMean = midMean / float64(len(midWindow[starId]))
				midNor := (midMean - historyMean) / historyStd
				midPrimaryScore := tailProb(midNor)
				if midPrimaryScore <= 0.5 {
					midPrimaryScore = 1 - midPrimaryScore
				}
           
				midEnergy := midPrimaryScore
				if midEnergy > ENERGYTHRESHOLD {
					curr.AnomalousList = append(curr.AnomalousList, true)
					curr.Anomalous = true
				} else {
					curr.AnomalousList = append(curr.AnomalousList, false)
				}

            	curr.MaxScore = maxf(curr.MaxScore, midEnergy)

				//**************************** DIFF_15 ****************************

				curr.AlgoNameList = append(curr.AlgoNameList, "DIFF_15")

				up_num := 0 
				for index := range midWindow[starId] {
					if index > 0 {
						diff := midWindow[starId][index].Mag - midWindow[starId][index - 1].Mag
						if diff < 0 {
							up_num++
						}
					}
				}

				if up_num * 5 >= len(midWindow[starId]) * 4 {
					curr.AnomalousList = append(curr.AnomalousList, true)
					curr.Anomalous = true
				} else {
					curr.AnomalousList = append(curr.AnomalousList, false)
				}
 
            	curr.MaxScore = maxf(curr.MaxScore, float64(up_num) / float64(len(midWindow[starId])))
			}

            if len(longWindow[starId]) == LONGWIDTH {
				
				//**************************** NFD_45 ****************************
            	
				curr.AlgoNameList = append(curr.AlgoNameList, "NFD_45")

				longMean := 0.0
				for _, s := range longWindow[starId] {
					longMean = longMean + s.Mag
				}
				longMean = longMean / float64(len(longWindow[starId]))
				longNor := (longMean - historyMean) / historyStd
				longPrimaryScore := tailProb(longNor)
				if longPrimaryScore <= 0.5 {
					longPrimaryScore = 1 - longPrimaryScore
				}

				longEnergy := longPrimaryScore
				if longEnergy > ENERGYTHRESHOLD {
					curr.AnomalousList = append(curr.AnomalousList, true)
					curr.Anomalous = true
				} else {
					curr.AnomalousList = append(curr.AnomalousList, false)
				}

				curr.MaxScore = maxf(curr.MaxScore, longEnergy)

				//**************************** DIFF_45 ****************************
            	
				curr.AlgoNameList = append(curr.AlgoNameList, "DIFF_45")

				up_num := 0 
				for index := range longWindow[starId] {
					if index > 0 {
						diff := longWindow[starId][index].Mag - longWindow[starId][index - 1].Mag
						if diff < 0 {
							up_num++
						}
					}
				}

				if up_num * 5 >= len(longWindow[starId]) * 4 {
					curr.AnomalousList = append(curr.AnomalousList, true)
					curr.Anomalous = true
				} else {
					curr.AnomalousList = append(curr.AnomalousList, false)
				}

            	curr.MaxScore = maxf(curr.MaxScore, float64(up_num) / float64(len(longWindow[starId])))   	
			}

			if curr.Anomalous {
				longFlagWindow[starId] = append(longFlagWindow[starId], true)
			} else {
				longFlagWindow[starId] = append(longFlagWindow[starId], false)
			}
			
		} else {
			curr.MaxScore = 0.0
			curr.AlgoNameList = append(curr.AlgoNameList, "NFD_5")
			curr.AlgoNameList = append(curr.AlgoNameList, "DIFF_5")
			curr.AlgoNameList = append(curr.AlgoNameList, "NFD_15")
			curr.AlgoNameList = append(curr.AlgoNameList, "DIFF_15")
			curr.AlgoNameList = append(curr.AlgoNameList, "NFD_45")
			curr.AlgoNameList = append(curr.AlgoNameList, "DIFF_45")
			curr.AnomalousList = append(curr.AnomalousList, false)
			curr.AnomalousList = append(curr.AnomalousList, false)
			curr.AnomalousList = append(curr.AnomalousList, false)
			curr.AnomalousList = append(curr.AnomalousList, false)
			curr.AnomalousList = append(curr.AnomalousList, false)
			curr.AnomalousList = append(curr.AnomalousList, false)
			curr.AnomalyName = ""
			longFlagWindow[starId] = append(longFlagWindow[starId], false)
		}

		resultChannel <- curr

        longFlagWindowLock[starId].Lock()
		if len(longFlagWindow[starId]) > LONGWIDTH {
			longFlagWindow[starId] = longFlagWindow[starId][1:]
		}
		longFlagWindowLock[starId].Unlock()
	}
DONE:
	workerWaitGroup.Done()
}

//********************* server  ***********************

func indexPageHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, indexPage)
}

func monitorPageHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, monitorPage)
}

func ajaxHandler(w http.ResponseWriter, r *http.Request) {
	ajaxResult.Lock()

	aS := ajaxResult

	ajaxResult.Unlock()

	aSjson, err := json.Marshal(aS)
	if err == nil {
		w.Write(aSjson)
	}
}

func anomalyHandler(w http.ResponseWriter, r *http.Request) {
	anomalousList := make([]Star, 0)

	resultListLock.Lock()
	for i := 0; i < 30; i++ { //返回前30颗异常星
		if resultList[i].Anomalous {
			historyWindowLock[resultList[i].StarId].Lock()
			anomalousList = append(anomalousList, historyWindow[resultList[i].StarId][len(historyWindow[resultList[i].StarId]) - 1])
			historyWindowLock[resultList[i].StarId].Unlock()
		}
	}
	resultListLock.Unlock()

	aSjson, err := json.Marshal(anomalousList)
	if err == nil {
		w.Write(aSjson)
	}
}

func starHandler(w http.ResponseWriter, r *http.Request) {
	starId, _ := strconv.ParseInt(r.URL.Query().Get("id"), 10, 64)
	starList :=  make([]Star, 0)

	historyWindowLock[starId].Lock()
	if len(historyWindow[starId]) > LONGWIDTH {
		starList = historyWindow[starId][len(historyWindow[starId]) - LONGWIDTH:]
	} else {
		starList = historyWindow[starId]
	}
	historyWindowLock[starId].Unlock()

	aSjson, err := json.Marshal(starList)
	if err == nil {
		w.Write(aSjson)
	}
}
//********************* main function ***********************

func main() {
	runtime.GOMAXPROCS(8)
	
	resultList = make([]Result, NUMOFSTARS)
	flag.BoolVar(&useTemplate, "usetemplate", false, "use template")
	flag.BoolVar(&useDebugCache, "usedebugcache", false, "use debug cache")
	flag.BoolVar(&noCache, "nocache", false, "don't cache when exit")
	flag.BoolVar(&noFileOutput, "nofileoutput", false, "don't output a file every step")
	
	flag.Parse()

	//read index html
	indexPageByte, err := ioutil.ReadFile("../static/html/index.html")
	if err == nil {
		indexPage = string(indexPageByte)
	} else {
		panic(err)
	}

    //read monitor html
	monitorPageByte, err := ioutil.ReadFile("../static/html/monitor.html")
	if err == nil {
		monitorPage = string(monitorPageByte)
	} else {
		panic(err)
	}

	createChannels()
	loadRecentData()
	loadTemp()

	masterWaitGroup.Add(2)
	go loaderAndCleaner()
	go dispatcher()

	for i := 1; i <= NUMOFSTARS; i++ {
		workerWaitGroup.Add(1)
		go worker(i)
	}
    
	http.Handle("/css/", http.FileServer(http.Dir("../static")))
	http.Handle("/data/", http.FileServer(http.Dir("../static")))
	http.Handle("/fonts/", http.FileServer(http.Dir("../static")))
	http.Handle("/html/", http.FileServer(http.Dir("../static")))
	http.Handle("/images/", http.FileServer(http.Dir("../static")))
	http.Handle("/js/", http.FileServer(http.Dir("../static")))
	
	http.HandleFunc("/", indexPageHandler)
	http.HandleFunc("/monitor", monitorPageHandler)
	http.HandleFunc("/ajax", ajaxHandler)
	http.HandleFunc("/anomalystarlist", anomalyHandler)
	http.HandleFunc("/star", starHandler)

	go http.ListenAndServe(":8080", nil)
	log.Println("监控服务器已启动......")

	for {
		fileName, active := <-fileNameChannel
		if !active {
			goto DONE
		}
		log.Println("时间戳: " + fileName)

		lineCount, active := <-lineCountChannel
		if !active {
			goto DONE
		}
		//log.Println("数据总行数: " + strconv.Itoa(lineCount))
		
        resultListLock.Lock()

		for i := 0; i < lineCount; i++ {
			resultData, active := <-resultChannel
			if !active {
				goto DONE
			}
			resultList[i] = resultData
			//outputFile.WriteString(strconv.Itoa(resultData.StarId) + " " + strconv.FormatFloat(float64(resultData.Score), 'f', 5, 32) + "\n")
		}

		log.Println("所有分析结果已获取，排序中......")
		sort.Sort(ResultArray(resultList)[:lineCount])
		log.Println("排序已完成......")
		anomaly_num := 0
		for i := 0; i < lineCount; i++ {
			if resultList[i].Anomalous {
				anomaly_num++
			}
		}

		log.Printf("%c[1;40;31m异常星数目:%s%c[0m", 0x1B, strconv.Itoa(anomaly_num), 0x1B)

		timePart := strings.Split(fileName, "_")
		timeStr := timePart[3] + ":" + timePart[4] + ":" + timePart[5]

		ajaxResult.Lock()

		ajaxResult.FirstStarId = resultList[0].StarId
		ajaxResult.SecondStarId = resultList[1].StarId

		ajaxResult.FirstAnomalyName = resultList[0].AnomalyName
		ajaxResult.SecondAnomalyName = resultList[1].AnomalyName
        ajaxResult.AlgoNameList = resultList[0].AlgoNameList
		ajaxResult.FirstAnomalousList = resultList[0].AnomalousList
		ajaxResult.SecondAnomalousList = resultList[1].AnomalousList

		resultListLock.Unlock()

		ajaxResult.TimeList = append(ajaxResult.TimeList, timeStr)
		if len(ajaxResult.TimeList) > LONGWIDTH {
			ajaxResult.TimeList = ajaxResult.TimeList[1:]
		}

		longWindowLock[ajaxResult.FirstStarId].Lock()
		longWindowLock[ajaxResult.SecondStarId].Lock()

        ajaxResult.FirstList = make([]float64, 0)
        for i := 0; i < len(longWindow[ajaxResult.FirstStarId]); i++ {
			ajaxResult.FirstList = append(ajaxResult.FirstList, longWindow[ajaxResult.FirstStarId][i].Mag)
		}
		
		ajaxResult.SecondList = make([]float64, 0)
		for i := 0; i < len(longWindow[ajaxResult.SecondStarId]); i++ {
			ajaxResult.SecondList = append(ajaxResult.SecondList, longWindow[ajaxResult.SecondStarId][i].Mag)
		}
		
		longWindowLock[ajaxResult.FirstStarId].Unlock()
		longWindowLock[ajaxResult.SecondStarId].Unlock()

		longFlagWindowLock[ajaxResult.FirstStarId].Lock()
		longFlagWindowLock[ajaxResult.SecondStarId].Lock()

		ajaxResult.FirstFlagList = longFlagWindow[ajaxResult.FirstStarId]
		ajaxResult.SecondFlagList = longFlagWindow[ajaxResult.SecondStarId]

		longFlagWindowLock[ajaxResult.FirstStarId].Unlock()
		longFlagWindowLock[ajaxResult.SecondStarId].Unlock()

		ajaxResult.Unlock()

		log.Println("检测结果已发送至监控界面\n")

        if !noFileOutput {
			//write to file
			outputFile, _ := os.Create("../result/" + fileName)
			for i := 0; i < lineCount; i++ {
				outputFile.WriteString(strconv.Itoa(resultList[i].StarId) + " " + strconv.FormatFloat(float64(resultList[i].MaxScore), 'f', 5, 32) + "\n")
			}
			log.Println("检测结果已导出至本地磁盘")
			outputFile.Close()
		}
	}
DONE:
	masterWaitGroup.Wait()
	workerWaitGroup.Wait()
	log.Println("所有worker已退出")
	log.Println("工作线程清理完毕")
    
	if noCache {
		log.Println("已设置为不对数据进行缓存")
	} else {
		saveRecentData()
	}

	log.Println("程序已退出")
}
