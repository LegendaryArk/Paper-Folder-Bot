#pragma once
#include "vex.h"

class Motor : public vex::motor
{
private:
	double tgtVel = 0;

public:
	Motor(int32_t port, bool reverse = false);

	void setVelocity(double velocity, vex::velocityUnits units);
	bool isStalling(double velThreshold = 40, double currentThreshold = 50);

	const static int TPR = 960;
};