#pragma once
#include "vex.h"
#include "motor.h"
#include "pid_controller.h"

class Hardstop : public Motor
{
private:
	PidController pid;

	double minFoldPos;
	double maxFoldPos;

	double maxEnc;

	double tgtPos = 0;
	double maxPower = 50;

	bool calibrated = false;

	vex::timer& timer;

public:
	Hardstop(Motor& motor, double kp, double ki, double kd, double minFoldPos, double maxFoldPos, double maxEnc, vex::timer& timer);

	void setTarget(double foldPos, double maxPower = 60);

	void update();

	bool isSettled(double posThreshold = 3, double velThreshold = 20);

	void calibrate();
	inline bool isCalibrated() const
		{ return calibrated; };

	inline double getMinFoldPos() const
		{ return minFoldPos; };
	inline double getMaxFoldPos() const
		{ return maxFoldPos; };
};