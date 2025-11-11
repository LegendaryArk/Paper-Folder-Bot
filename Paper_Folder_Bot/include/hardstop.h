#pragma once
#include "vex.h"
#include "motor.h"
#include "pid_controller.h"

class Hardstop {
	private:
		Motor& motor;
		PidController pid;

		double tgtPos = 0;
		double maxPower = 50;

		vex::timer& timer;

	public:
		Hardstop(Motor& motor, double kp, double ki, double kd, vex::timer& timer);

		void setTarget(double foldPos, double maxPower = 50);

		void update();

		bool isSettled();
};