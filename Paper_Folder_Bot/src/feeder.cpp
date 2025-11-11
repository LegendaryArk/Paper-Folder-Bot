#include "feeder.h"

using namespace vex;

Feeder::Feeder(Motor& motor, distance& distanceSensor, vex::timer& timer)
	: motor(motor), distanceSensor(distanceSensor), timer(timer) {}

bool Feeder::isIndexingPaper() {
	return distanceSensor.objectDistance(distanceUnits::mm) < PAPER_DETECTION_DISTANCE_MM;
}

void Feeder::index(double power, double timeout) {
	motor.spin(directionType::fwd, power, velocityUnits::pct);
	
	double st = timer.value();

	while (!isIndexingPaper() && timer.value() - st < timeout) {}
	while (isIndexingPaper() && timer.value() - st < timeout) {}

	motor.spin(directionType::rev, 10, velocityUnits::pct);
	wait(100, msec);
	motor.stop();
}