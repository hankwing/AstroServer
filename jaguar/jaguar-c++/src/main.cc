#include <Jaguar.h>
#include <cmath>

using namespace Jaguar;

Result singleValueDetect(int starNum, Cache* cc) {
	float currentValue = cc->getLastOne(starNum);
	if (starNum == 10000) {
		return Result(true, "singlePointAnomaly");
	} else {
		return Result(false, NULL);
	}
}

/*
Result normalizedFeatureDeviation(int starNum, Cache* cc) {
	static const float warnThreshold = 0.008;
    static const int longWindowLen = 2000;
	static const int shortWindowLen = 40;

	vector<float> longWindow = cc->getLastMany(starNum, longWindowLen);
    vector<float> shortWindow = cc->getLastMany(starNum, shortWindowLen);
	
	float longAverage = 0.0;
	for (auto& elem: longWindow) {
		longAverage += elem;
	}
	longAverage /= longWindowLen;
	
	float longStd = 0.0;
	for (auto& elem: longWindow) {
		longStd += (elem - longAverage) * (elem - longAverage);
	}
	longStd /= longWindowLen;
    longStd = sqrt(longStd);

	float shortAverage = 0.0;
	for (auto& elem: shortWindow) {
		shortAverage += elem;
	}
	shortAverage /= shortWindowLen;

	nor = (shortAverage - longAverage) / longStd;

	float primaryScore = 0.5 * (1 - erf(nor / sqrt(2)));
	if primaryScore >= 0.5 {
		primaryScore = 1 - primaryScore;
	}

	if primaryScore < warnThreshold {
		return Result(true, "shortWindowAnomaly");
	} else {
		return Result(false, NULL);
	}
}
*/

int main() {
	App app("../pipe/namedpipe", 176000, 2000);
	app.registerAlgorithm(singleValueDetect, "SingleValueDetect");
	//app.registerAlgorithm(normalizedFeatureDeviation,"NormalizedFeatureDeviation");
	app.run();
}