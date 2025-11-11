#pragma once
#include "vex.h"

class PidController {
	double kp, ki, kd;

	vex::timer& timer;

	double prevError = 0;
	double integral = 0;
	double prevTime = 0;

	double outputMax = 1, outputMin = -1;

	public:
	PidController(double kp, double ki, double kd, vex::timer& timer, double outputMax = 1, double outputMin = -1);

	double calculate(double error);
	double calculate(double target, double current);
};