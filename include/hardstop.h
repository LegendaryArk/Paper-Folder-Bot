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

		bool calibrated = false;

		vex::timer& timer;

	public:
		Hardstop(Motor& motor, double kp, double ki, double kd, vex::timer& timer);

		void setTarget(double foldPos, double maxPower = 60);

		void update();

		bool isSettled(double posThreshold = 3, double velThreshold = 20) const;

		void calibrate();
		inline bool isCalibrated() const { return calibrated; };

		inline void stop() { motor.stop(); };

		const static double MIN_FOLD_POS_CM = 6.99;
		const static double MAX_FOLD_POS_CM = 17.46;
};