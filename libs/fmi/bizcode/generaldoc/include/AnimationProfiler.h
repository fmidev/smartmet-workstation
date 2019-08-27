#pragma once

#include <vector>
#include <chrono>
#include <sstream>
#include <numeric>
#include <algorithm>

typedef std::chrono::time_point<std::chrono::steady_clock> profilePoint;

class NFmiAnimationData;

class AnimationProfiler {

	profilePoint lastTick;

	std::vector<NFmiAnimationData> settings;

	std::vector<std::chrono::high_resolution_clock::duration> data;

public:

	AnimationProfiler(): lastTick(std::chrono::high_resolution_clock::now() ), settings(), data() {};

	void Reset() {
		data.clear();
		settings.clear();

		lastTick = std::chrono::high_resolution_clock::now();
	}

	std::vector< NFmiAnimationData >& getSettings() {
		return settings;
	}

	size_t dataCount() {
		return data.size();
	}

	void Tick() {

		auto currTime = std::chrono::high_resolution_clock::now();

		if (lastTick != profilePoint()) {

			auto t = currTime - lastTick;

			data.push_back(t);

		}

		lastTick = currTime;
	}




	void Report() {

		if (!data.size()) {

			CatLog::logMessage("No data", CatLog::Severity::Info, CatLog::Category::Visualization);
			return;
		}
 		auto ret = std::stringstream{};

		std::vector < std::chrono::duration<double,std::milli> > datams;

		std::transform(begin(data), end(data), std::back_inserter(datams), [](const std::chrono::nanoseconds &a) {
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(a);
			});

		size_t pos = 1;
		constexpr int lineLen = 10;

		std::stringstream dataLine;

		dataLine << std::setprecision(2) << std::fixed;


		for (size_t i = 0; i < datams.size(); i++) {
			auto a = datams[i];
			dataLine << i <<" : "<< a.count() << "ms | ";
			if (pos % lineLen == 0 || i == datams.size() - 1) {
				CatLog::logMessage(dataLine.str(), CatLog::Severity::Info, CatLog::Category::Visualization);
				pos = 1;
				dataLine = std::stringstream{};

				dataLine << std::setprecision(2) << std::fixed;
			}
			else pos++;
		}

		ret << "Samples: " << data.size() << "    ";

		std::chrono::duration<double> total(0);

		total = std::accumulate(begin(datams), end(datams), total);

		ret << "Total: " << total.count() << "s    ";

		std::sort(begin(datams), end(datams));

		double median = datams[datams.size() / 2].count();

		double dataMin = datams.front().count();

		double dataMax = datams.back().count();

		ret << "Min: " << dataMin << "ms , Max: " << dataMax << "ms, median: " << median << "ms    ";

		double stddev = std::accumulate(begin(datams), end(datams), 0, [=](double a, const std::chrono::duration<double, std::milli>  &b) {
			return (a + (b.count() - median) * (b.count() - median) );
			});
		stddev = std::sqrt( stddev/(data.size()-1) );

	

		ret << "Std. deviation: " << stddev << "ms";


		CatLog::logMessage(ret.str(), CatLog::Severity::Info, CatLog::Category::Visualization);


		constexpr int binCount = 16;

		double binSize = (dataMax - dataMin) / binCount;

		double currBinVal = dataMin + binSize;

		int currBin = 0;

		std::array<size_t, binCount> bins = { 0 };


		for (auto&& p : datams) {

			double val = p.count();
			while ( val > currBinVal) {
				currBinVal += binSize;
				currBin++;
			}

			bins[currBin>=binCount?binCount-1:currBin]++;


		}

		size_t maxCount = 0;
		for (int i = 0; i < binCount; i++)
			if (bins[i] > maxCount) maxCount = bins[i];

		constexpr int maxBar = 40;

		for (int i = 0; i < binCount; i++) {
			std::stringstream line;
			auto binVal = dataMin + binSize * i;
			line << "> "  << std::setfill('0') << std::setw(7) << binVal << "ms : " << std::setw(3) << bins[i] << " | ";
			int barLen = double(bins[i]) / maxCount * maxBar;
			if (barLen > 0)
				for (int j = 0; j < barLen; j++)
					line << " # ";
			else if (bins[i] > 0)
				line << " * ";
			CatLog::logMessage(line.str(), CatLog::Severity::Info, CatLog::Category::Visualization);
		}
	}

};