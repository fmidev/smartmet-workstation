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

	AnimationProfiler(): lastTick(), settings(), data() {};

	void Reset() {
		data.clear();
		settings.clear();

		lastTick = profilePoint();
	}

	std::vector< NFmiAnimationData >& getSettings() {
		return settings;
	}

	void Tick() {

		auto currTime = std::chrono::high_resolution_clock::now();

		if (lastTick != profilePoint()) {

			auto t = currTime - lastTick;

			data.push_back(t);

		}

		lastTick = currTime;
	}




	std::string report() {

		if (!data.size() ) return "No data collected";
 		auto ret = std::stringstream{};

		std::vector < std::chrono::duration<double,std::milli> > datams;

		std::transform(begin(data), end(data), std::back_inserter(datams), [](const std::chrono::nanoseconds &a) {
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(a);
			});

		ret << "Samples: " << data.size() << "    ";

		std::chrono::duration<double> total(0);

		total = std::accumulate(begin(datams), end(datams), total);

		ret << "Total: " << total.count() << "s    ";

		std::sort(begin(datams), end(datams));

		double median = datams[datams.size() / 2].count();

		ret << "Min: " << datams.front().count() << "ms , Max: " << datams.back().count() << "ms, median: " << median << "ms    ";

		double stddev = std::accumulate(begin(datams), end(datams), 0, [=](double a, const std::chrono::duration<double, std::milli>  &b) {
			return (a + (b.count() - median) * (b.count() - median) );
			});
		stddev = std::sqrt( stddev/(data.size()-1) );

	

		ret << "Std. deviation: " << stddev << "ms";

		return ret.str();
	}

};