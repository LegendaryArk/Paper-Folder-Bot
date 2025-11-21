#pragma once

#include "vex.h"
#include "motor.h"

class Feeder {
	private:
		Motor& motor;
		vex::distance& distanceSensor;
		vex::timer& timer;

		vex::controller controller;

		const double PAPER_DETECTION_DISTANCE_MM = 80;
	
		bool isIndexingPaper() const;

	public:
		Feeder(Motor& motor, vex::distance& distanceSensor, vex::timer& timer, vex::controller& controller);

		bool index(double power = 50, double timeout = 10, bool& paused = *(new bool(false)));

		inline void stop() { motor.stop(); };
};