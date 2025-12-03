#include "pid_controller.h"
#include "util.h"

PidController::PidController(double kp, double ki, double kd, vex::timer& timer, double integralRange, bool resetIntegralOnCross, double outputMax, double outputMin)
			: kp(kp), ki(ki), kd(kd), timer(timer), integralRange(integralRange), resetIntegralOnCross(resetIntegralOnCross), outputMax(outputMax), outputMin(outputMin) {}

double PidController::calculate(double error)
{
	static double prevTime = 0;
	static double integral = 0;
	static double prevError = 0;

	double dt = timer.value() - prevTime;
	prevTime = timer.value();

	if (fabs(error) < integralRange)
		integral += error * dt;
	else
		integral = 0;
	if (resetIntegralOnCross && Util::sgn(error) != Util::sgn(prevError))
		integral = 0;
	
	double derivative = (error - prevError) / dt;
	prevError = error;

	return Util::clamp(kp * error + ki * integral + kd * derivative, outputMin, outputMax);
}
double PidController::calculate(double target, double current)
{
	return calculate(target - current);
}