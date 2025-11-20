#pragma once
#include "vex.h"

class Motor : public vex::motor {
	private:
		double tgtVel = 0;

		vex::timer& timer;

		const int TPR = 960;
	
	public:
		Motor(int32_t port, vex::timer& timer, bool reverse = false);

		void setVelocity(double velocity, vex::velocityUnits units);

		bool isStalling(double velThreshold = 40, double currentThreshold = 50);
};