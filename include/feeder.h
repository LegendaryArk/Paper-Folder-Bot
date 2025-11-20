#pragma once

#include "vex.h"
#include "motor.h"

class Feeder {
	private:
		Motor& motor;
		vex::distance& distanceSensor;
		vex::timer& timer;

		const double PAPER_DETECTION_DISTANCE_MM = 50;
	
		bool isIndexingPaper() const;

	public:
		Feeder(Motor& motor, vex::distance& distanceSensor, vex::timer& timer);

		bool index(double power = 50, double timeout = 10);

		inline void stop() { motor.stop(); };
};