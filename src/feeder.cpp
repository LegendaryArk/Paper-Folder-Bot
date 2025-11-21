#include "feeder.h"

using namespace vex;

Feeder::Feeder(Motor& motor, distance& distanceSensor, vex::timer& timer, vex::controller& controller)
	: motor(motor), distanceSensor(distanceSensor), timer(timer), controller(controller) {
	motor.setBrake(brakeType::coast);	
}

bool Feeder::isIndexingPaper() const {
	return distanceSensor.objectDistance(distanceUnits::mm) < PAPER_DETECTION_DISTANCE_MM;
}

bool Feeder::index(double power, double timeout, bool& paused) {
	motor.setMaxTorque(100, percentUnits::pct);
	motor.setVelocity(power, velocityUnits::rpm);
	motor.spin(directionType::fwd);
	
	double st = timer.value();

	int cnt = 0;
	bool trayEmpty = false;
	while (cnt < 20 && !trayEmpty) {
		if (isIndexingPaper())
			cnt++;
		
		if (timer.value() - st > timeout && cnt == 0)
			trayEmpty = true;
		
		if (controller.ButtonEUp.pressing()) {
			motor.stop();
			paused = true;
			while (controller.ButtonEUp.pressing()) {}
			while (!controller.ButtonEUp.pressing()) {}
			while (controller.ButtonEUp.pressing()) {}
			return true;
		}
		
		wait(50, msec);
	}

	if (trayEmpty) {
		motor.stop();
		return false;
	}

	motor.stop();
	wait(100, msec);

	motor.setMaxTorque(30, percentUnits::pct);
	motor.setVelocity(40, velocityUnits::pct);
	motor.spin(directionType::rev);
	wait(700, msec);
	motor.stop();
	return true;
}