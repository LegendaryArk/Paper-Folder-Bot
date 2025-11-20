#include "feeder.h"

using namespace vex;

Feeder::Feeder(Motor& motor, distance& distanceSensor, vex::timer& timer)
	: motor(motor), distanceSensor(distanceSensor), timer(timer) {
	motor.setBrake(brakeType::coast);	
}

bool Feeder::isIndexingPaper() const {
	return distanceSensor.objectDistance(distanceUnits::mm) < PAPER_DETECTION_DISTANCE_MM;
}

bool Feeder::index(double power, double timeout) {
	motor.setVelocity(power, velocityUnits::rpm);
	motor.spin(directionType::fwd);
	
	double st = timer.value();

	int cnt = 0;
	while (cnt < 5 && timer.value() - st < timeout) {
		if (isIndexingPaper())
			cnt++;
		else
			cnt = 0;
		
		wait(100, msec);
	}

	printf("Indexing complete\n");

	if (timer.value() - st >= timeout) {
		motor.stop();
		return false;
	}

	motor.spin(directionType::rev, 10, velocityUnits::pct);
	wait(100, msec);
	motor.stop();
	return true;
}