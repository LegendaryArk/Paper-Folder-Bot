#pragma once

#include "vex.h"
#include "motor.h"

class Feeder : public Motor
{
private:
	vex::distance& distanceSensor;
	vex::timer& timer;

	const static double PAPER_DETECTION_DISTANCE_MM = 80;

public:
	Feeder(Motor& motor, vex::distance& distanceSensor, vex::timer& timer);

	bool isIndexingPaper() const;

	bool index(double power);
};