#pragma once

#include <vector>
#include <chrono>
#include <sstream>
#include <numeric>
#include <algorithm>

#include "NFmiMetTime.h"

typedef std::chrono::time_point<std::chrono::steady_clock> profilePoint;

class NFmiAnimationData;

class AnimationProfiler {

	profilePoint lastTick;
	std::vector<NFmiAnimationData> settings;
	std::vector<std::chrono::high_resolution_clock::duration> data;
	std::vector<NFmiMetTime> dataTimes;

public:

	AnimationProfiler(): lastTick( ), settings(), data(), dataTimes() {};

	void Reset() {
		data.clear();
		dataTimes.clear();
		settings.clear();

		lastTick = profilePoint();
	}

	std::vector< NFmiAnimationData >& getSettings() {
		return settings;
	}

	size_t dataCount() {
		return data.size();
	}

	size_t tickCount() {
		return dataTimes.size();
	}

	void Tick(const NFmiMetTime&tb) {

		auto currTime = std::chrono::high_resolution_clock::now();

		dataTimes.push_back(tb);

		if (lastTick != profilePoint()) {
			auto t = currTime - lastTick;
			data.push_back(t);
		}

		lastTick = currTime;
	}

	typedef std::chrono::duration<double, std::milli> doubleMS;
	
	void ReportHistogram(double& dataMax, double& dataMin, std::vector<doubleMS>& data)
	{
		constexpr int binCount = 16;
		CatLog::logMessage("Profiling histogram:", CatLog::Severity::Info, CatLog::Category::Visualization);
		
		if (data.size() < binCount) return;

		double binSize = (dataMax - dataMin) / binCount;
		double currBinVal = dataMin + binSize;
		int currBin = 0;

		std::array<size_t, binCount> bins = { 0 };

		for (auto&& p : data) {
			double val = p.count();
			while (val > currBinVal) {
				currBinVal += binSize;
				currBin++;
			}
			bins[currBin >= binCount ? binCount - 1 : currBin]++;
		}

		size_t maxCount = 0;
		for (int i = 0; i < binCount; i++)
			if (bins[i] > maxCount) maxCount = bins[i];

		constexpr int maxBar = 40;

		for (int i = 0; i < binCount; i++) {
			std::stringstream line;
			auto binVal = dataMin + binSize * i;
			line << "> " << std::setfill('0') << std::setw(7) << binVal << "ms : " << std::setw(3) << bins[i] << " | ";
			int barLen = static_cast<int>(static_cast<double>(bins[i]) / maxCount * maxBar);
			if (barLen > 0)
				for (int j = 0; j < barLen; j++)
					line << " # ";
			else if (bins[i] > 0)
				line << " * ";
			
			CatLog::logMessage(line.str(), CatLog::Severity::Info, CatLog::Category::Visualization);
		}
	}

	void Report() {
		if (!data.size()) {
			CatLog::logMessage("No data", CatLog::Severity::Info, CatLog::Category::Visualization);
			return;
		}
 		auto ret = std::stringstream{};

		std::vector <doubleMS> datams;

		std::transform(begin(data), end(data), std::back_inserter(datams), [](const std::chrono::nanoseconds &a) {
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(a);
			});

		size_t pos = 1;
		constexpr int lineLen = 5;

		std::stringstream dataLine;
		CatLog::logMessage("Profiling: measures how long each timestep takes to visualize.", CatLog::Severity::Info, CatLog::Category::Visualization);

		dataLine << std::setprecision(0) << std::fixed;

		for (size_t i = 0; i < datams.size(); i++) {
			auto a = datams[i];
			dataLine << dataTimes[i] <<" : "<< a.count() << "ms | ";
			if (pos % lineLen == 0 || i == datams.size() - 1) {
				CatLog::logMessage(dataLine.str(), CatLog::Severity::Info, CatLog::Category::Visualization);
				pos = 1;
				dataLine = std::stringstream{};
				dataLine << std::setprecision(0) << std::fixed;
			}
			else pos++;
		}

		ret << "Samples: " << data.size() << " ";

		std::chrono::duration<double> total(0);
		total = std::accumulate(begin(datams), end(datams), total);
		ret << "Total: " << total.count() << "s  ";

		std::sort(begin(datams), end(datams));

		double median = datams[datams.size() / 2].count();
		double dataMin = datams.front().count();
		double dataMax = datams.back().count();
		ret << "Min: " << dataMin << "ms  Max: " << dataMax << "ms  Median: " << median << "ms  ";

		double stddev = std::accumulate<std::vector<doubleMS>::iterator, double>
			(begin(datams), end(datams), 0, [=](double a, const std::chrono::duration<double, std::milli>  &b) {
			return (a + (b.count() - median) * (b.count() - median) );
			});
		stddev = std::sqrt( stddev/(data.size()-1) );
		ret << "Standard deviation: " << stddev << "ms ";

		CatLog::logMessage(ret.str(), CatLog::Severity::Info, CatLog::Category::Visualization);

		ReportHistogram(dataMax, dataMin, datams);
	}

};