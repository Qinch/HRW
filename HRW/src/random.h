
#ifndef RANDOM_H_
#define RANDOM_H_

#include <random>

class Random {
private:
	std::mt19937 _engine;

public:
	Random(): _engine(std::random_device()()) {}
	double value()
   	{
		double R =
		(double)(std::mt19937::max() - std::mt19937::min()) + 1.0;
		return (double)(_engine() - _engine.min()) / R;
	}

	double getrand() 
	{
		   	return value(); 
	}
};

#endif 

