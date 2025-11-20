#include "motor.h"

using namespace vex;

Motor::Motor(int32_t port, vex::timer& timer, bool reverse)
	: motor(port, reverse), timer(timer) {}

void Motor::setVelocity(double velocity, velocityUnits units) {
	tgtVel = velocity;
	motor::setVelocity(velocity, units);
}

bool Motor::isStalling(double velThreshold, double currentThreshold) {
	// printf("Target Vel: %.2f, Actual Vel: %.2f, Current: %.2f\n", tgtVel, motor::velocity(vex::velocityUnits::rpm), motor::current(percentUnits::pct));
	return fabs(tgtVel - fabs(motor::velocity(vex::velocityUnits::rpm))) > velThreshold && motor::current(percentUnits::pct) > currentThreshold;
}