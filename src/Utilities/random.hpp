#ifndef POPHEAD_UTILITIES_RANDOM_H_
#define POPHEAD_UTILITIES_RANDOM_H_

#include <random>

namespace ph {

class Random {
public:
	static float generateNumber(float min, float max);

private:
	static std::default_random_engine mEngine;
};

}

#endif
