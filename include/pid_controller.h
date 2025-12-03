#pragma once
#include "vex.h"

class PidController
{
private:
	double kp, ki, kd;

	vex::timer& timer;

	double integralRange = 15;
	bool resetIntegralOnCross = true;

	double outputMax = 1, outputMin = -1;

public:
	PidController(double kp, double ki, double kd, vex::timer& timer, double integralRange = 15, bool resetIntegralOnCross = true, double outputMax = 1, double outputMin = -1);

	double calculate(double error);
	double calculate(double target, double current);
};