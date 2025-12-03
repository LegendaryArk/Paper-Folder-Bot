#pragma once

#include "vex.h"
#include "motor.h"

class Rollers : public Motor
{
private:
	vex::optical& opticalSensor;

	vex::timer& timer;

	const double TIME_THRESHOLD_SEC = 3;

public:
	Rollers(Motor& motor, vex::optical& opticalSensor, vex::timer& timer);

	bool isPaperDetected() const;
	bool isClear() const;
	bool isOutputTrayFull() const;

	void spin();
};