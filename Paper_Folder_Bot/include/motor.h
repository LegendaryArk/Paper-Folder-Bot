#pragma once
#include "vex.h"

class Motor : public vex::motor {
	private:
		double gearRatio = 1;

		double prevEncPos = 0;
		double prevTime = 0;

		vex::timer& timer;

		const int TPR = 960;
	
	public:
		Motor(int32_t port, vex::timer& timer, bool reverse = false, double gearRatio = 1);

		double getDeltaPosition();
		bool isStalling(double threshold = 5);
};