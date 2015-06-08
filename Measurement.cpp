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
		times[measurementEpoch % numberOfMeasurements] = time;
	}

	uint64_t meanTime() const
	{
		uint64_t sumOfTimes = 0;
		uint8_t numberOfTimes = 0;
		for(int i = std::max(measurementEpoch - numberOfMeasurements, firstEpoch); i < measurementEpoch; i++)
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
		if(measurementEpoch - firstEpoch < numberOfMeasurements)
			return false;
		for(int i = measurementEpoch - numberOfMeasurements; i < measurementEpoch; i++)
			if(!times[i % numberOfMeasurements].empty())
				return false;
		return true;
	}

private:
	const unsigned firstEpoch;
	unsigned lastEpoch;
	MeasuredTime times[numberOfMeasurements];
};

struct compareAddress_v4
{
	bool operator () (const address_v4 & a, const address_v4 & b)
	{
		return a.to_ulong() < b.to_ulong();
	}
};

std::map<
	address_v4,
	std::map<std::string, MeasurementArray>,
	compareAddress_v4
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
		if(time == 0 && it->second.checkIfShouldBeDeleted())
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
