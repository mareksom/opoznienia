#include "Measurement.h"

#include <algorithm>

using boost::asio::ip::address_v4;

const int numberOfMeasurements = 10;

namespace {

unsigned measurementEpoch = 0;

class MeasuredTime
{
public:
	MeasuredTime(uint64_t time = 0) : time(time) { }
	bool empty() const { return time == 0; }
	void clear() { this->time = 0; }
	uint64_t getTime() const { return time; }
	void setTime(uint64_t time) { this->time = time; }

private:
	uint64_t time;
};

class MeasurementArray
{
public:
	MeasurementArray() : firstEpoch(measurementEpoch), lastEpoch(measurementEpoch) { }

	void addMeasurement(uint64_t time)
	{
		lastEpoch = measurementEpoch;
		times[measurementEpoch % numberOfMeasurements] = time;
	}

	uint64_t meanTime() const
	{
		if(checkIfShouldBeDeleted())
			return 0;
		uint64_t sumOfTimes = 0;
		uint8_t numberOfTimes = 0;
		unsigned i;
		if(measurementEpoch < firstEpoch + numberOfMeasurements)
			i = firstEpoch;
		else
			i = measurementEpoch - numberOfMeasurements;
		for(; i <= lastEpoch and i < measurementEpoch; i++)
		{
			const MeasuredTime & t = times[i % numberOfMeasurements];
			if(!t.empty())
			{
				sumOfTimes += t.getTime();
				numberOfTimes++;
			}
		}
		if(numberOfTimes == 0)
			return 0;
		return (sumOfTimes + numberOfTimes - 1) / numberOfTimes;
	}

	bool checkIfShouldBeDeleted() const
	{
		return lastEpoch + numberOfMeasurements <= measurementEpoch;
	}

private:
	const unsigned firstEpoch;
	unsigned lastEpoch;
	MeasuredTime times[numberOfMeasurements];
};

std::map<
	address_v4,
	std::map<std::string, MeasurementArray>
> measurements;

} // namespace


namespace Measurement {

void addMeasurement(address_v4 ip, const std::string & method, uint64_t time)
{
	measurements[ip][method].addMeasurement(time);
}

void skipMeasurements()
{
	measurementEpoch++;
}

std::map<std::string, uint64_t> getTimes(boost::asio::ip::address_v4 ip)
{
	std::map<std::string, MeasurementArray> & data = measurements[ip];
	std::map<std::string, uint64_t> result;
	for(auto it = data.begin(); it != data.end();)
	{
		uint64_t time = it->second.meanTime();
		if(time == 0 and it->second.checkIfShouldBeDeleted())
			data.erase(it++);
		else
		{
			result[it->first] = time;
			it++;
		}
	}
	return result;
}

} // namespace Measurement
