#include "vex.h"
#include "motor.h"
#include "feeder.h"
#include "hardstop.h"
#include "util.h"

vex::brain brain;
vex::controller controller;

Motor feederMotor(vex::PORT7, brain.Timer, true);
Motor rollers(vex::PORT1, brain.Timer, false);
Motor hardstop1Motor(vex::PORT6, brain.Timer, false);
Motor hardstop2Motor(vex::PORT8, brain.Timer, true);
vex::distance distanceSensor(vex::PORT2);
vex::optical opticalSensor(vex::PORT5);
vex::touchled touchLed(vex::PORT4);

Feeder feeder(feederMotor, distanceSensor, brain.Timer, controller);
Hardstop hardstop1(hardstop1Motor, 0.02, 0.015, 0.0013, 7.2, 17.1, 1352, brain.Timer);
Hardstop hardstop2(hardstop2Motor, 0.02, 0.015, 0.0015, 8.5, 17.4, 1262, brain.Timer);

void inputFoldLocations(double& fold1, double& fold2) {
    brain.Screen.clearScreen();

    brain.Screen.setCursor(5, 1);
    brain.Screen.setFont(vex::fontType::mono15);
    brain.Screen.print("Press Touch LED");

    double precision = 1.0;

    double* foldLocs[2] = {&fold1, &fold2};
    const vex::controller::button* btns[4] = {
        &controller.ButtonLUp,
        &controller.ButtonLDown,
        &controller.ButtonRUp,
        &controller.ButtonRDown
    };
    double pressSt[4] = {0};
    bool precisionDebounce[2] = {false};

    bool ledPressed = false, confirmPressed = false, controllerPressed = false, confirmed = false;
    bool resetPressed = false;
    while (!confirmed) {
        if (touchLed.pressing() && !ledPressed)
            ledPressed = true;
        if (!touchLed.pressing() && ledPressed)
            confirmed = true;
        
        if (controller.ButtonEUp.pressing() && !controllerPressed)
            controllerPressed = true;
        if (!controller.ButtonEUp.pressing() && controllerPressed)
            confirmed = true;
        
        if (brain.buttonCheck.pressing() && !confirmPressed)
            confirmPressed = true;
        if (!brain.buttonCheck.pressing() && confirmPressed)
            confirmed = true;

        if (controller.ButtonEDown.pressing() && !resetPressed) {
            resetPressed = true;
        } else if (!controller.ButtonEDown.pressing() && resetPressed) {
            fold1 = ceil(hardstop1.getMinFoldPos());
            fold2 = ceil(hardstop2.getMinFoldPos() + fold1);
            resetPressed = false;
        }

        for (int i = 0; i < 2; i++) {
            brain.Screen.setCursor(i + 1, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Fold %d: %.1f cm", i + 1, *foldLocs[i]);
        }

        brain.Screen.setCursor(3, 1);
        brain.Screen.clearLine();
        brain.Screen.print("Precision: %.1f", precision);

        for (int i = 0; i < 4; i++) {
            if (btns[i]->pressing() && !pressSt[i]) {
                pressSt[i] = brain.Timer.value();
            }
        }

        for (int i = 0; i < 4; i++) {
            if (btns[i]->pressing() && brain.Timer.value() - pressSt[i] >= 0.5) {
                if (i % 2)
                    *foldLocs[i / 2] -= precision;
                else if (*foldLocs[0] + *foldLocs[1] + precision < 28)
                    *foldLocs[i / 2] += precision;
                
                if (*foldLocs[1] - *foldLocs[0] < hardstop1.getMinFoldPos()) {
                    *foldLocs[((i / 2) + 1) % 2] = *foldLocs[i / 2] + copysign(hardstop1.getMinFoldPos(), 1 - i);
                }
            } else if (!btns[i]->pressing()) {
                if (brain.Timer.value() - pressSt[i] < 0.5) {
                    if (i % 2)
                        *foldLocs[i / 2] -= precision;
                    else if (*foldLocs[0] + *foldLocs[1] + precision < 28)
                        *foldLocs[i / 2] += precision;
                    
                    if (*foldLocs[1] - *foldLocs[0] < hardstop1.getMinFoldPos()) {
                        *foldLocs[((i / 2) + 1) % 2] = *foldLocs[i / 2] + copysign(hardstop1.getMinFoldPos(), 1 - i);
                    }
                }
                pressSt[i] = 0;
            }
        }
        
        if (controller.ButtonFUp.pressing() && !precisionDebounce[0]) {
            if (precision < 10)
                precision *= 10;
            precisionDebounce[0] = true;
        } else if (!controller.ButtonFUp.pressing() && precisionDebounce[0]) {
            precisionDebounce[0] = false;
        }
        if (controller.ButtonFDown.pressing() && !precisionDebounce[1]) {
            if (precision > 0.1)
                precision /= 10;
            precisionDebounce[1] = true;
        } else if (!controller.ButtonFDown.pressing() && precisionDebounce[1]) {
            precisionDebounce[1] = false;
        }

        fold1 = Util::clamp(fold1, hardstop1.getMinFoldPos(), hardstop1.getMaxFoldPos());
        fold2 = Util::clamp(fold2, hardstop2.getMinFoldPos() + hardstop1.getMinFoldPos(), hardstop2.getMaxFoldPos() + hardstop1.getMaxFoldPos());

        if (fold2 - fold1 < fold1) {
            brain.Screen.setCursor(6, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Folds overlap:");
            brain.Screen.setCursor(7, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Will cause 3 folds");
        } else {
            brain.Screen.setCursor(6, 1);
            brain.Screen.clearLine();
            brain.Screen.setCursor(7, 1);
            brain.Screen.clearLine();
        }

        wait(100, vex::msec);
    }
}

void displayError(int errorCode) {
    switch (errorCode) {
        case 0:
            brain.Screen.clearScreen();
            brain.Screen.setCursor(3, 1);
            brain.Screen.print("Folding Complete!");

            touchLed.setBlink(vex::color::green, 0.5, 0.5);
            brain.playSound(vex::tada);
            break;
        case 1:
            brain.Screen.clearScreen();
            brain.Screen.setCursor(3, 1);
            brain.Screen.print("Error: Paper Jammed!");

            touchLed.setBlink(vex::color::red, 0.5, 0.5);
            for (int i = 0; i < 3; i++) {
                brain.playSound(vex::alarm);
                wait(500, vex::msec);
            }
            break;
        case 2:
            brain.Screen.clearScreen();
            brain.Screen.setCursor(3, 1);
            brain.Screen.print("Error: Folding Timeout!");

            touchLed.setBlink(vex::color::red, 0.5, 0.5);
            for (int i = 0; i < 3; i++) {
                brain.playSound(vex::alarm);
                wait(500, vex::msec);
            }
            break;
    }
}

int main() {
    printf("\033[2J\n");

    int errorCode = 0;

    brain.Screen.setCursor(1, 1);
    brain.Screen.print("Calibrating...");
    while (!hardstop1.isCalibrated() || !hardstop2.isCalibrated()) {
        hardstop1.calibrate();
        hardstop2.calibrate();
        wait(50, vex::msec);
    }

	double fold1Location = ceil(hardstop1.getMinFoldPos());
    double fold2Location = ceil(hardstop2.getMinFoldPos() + fold1Location);
    inputFoldLocations(fold1Location, fold2Location);

    printf("Fold Locations: %.1f, %.1f\n", fold1Location, fold2Location);

    brain.Screen.clearLine(3);
    brain.Screen.clearLine(5);

    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Initializing...");

    hardstop1.setTarget(fold1Location);
    hardstop2.setTarget(fold2Location - fold1Location);

    while (!hardstop1.isSettled() || !hardstop2.isSettled()) {
        hardstop1.update();
        hardstop2.update();
        wait(20, vex::msec);
    }

    hardstop1.stop();
    hardstop2.stop();

    printf("%f, %f\n", hardstop1Motor.position(vex::rotationUnits::raw), hardstop2Motor.position(vex::rotationUnits::raw));

    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Folding...");

    rollers.setMaxTorque(100, vex::percentUnits::pct);
    rollers.setVelocity(80, vex::velocityUnits::rpm);
    rollers.spin(vex::directionType::fwd);

    int numPaper = 0;
    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Folding... %d", numPaper);

    bool paused = false;

    double indexTime = 0;
    bool isTrayEmpty = false;
    bool isJammed = false;
    while (!isTrayEmpty && !isJammed) {
        isTrayEmpty = !feeder.index(40, 3, paused);

        if (!paused && !isTrayEmpty) {
            indexTime = brain.Timer.value();

            numPaper++;
            brain.Screen.setCursor(4, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Folding... %d", numPaper);

            bool isClear = false, paperDetected = false;
            while (!isClear && brain.Timer.value() - indexTime < 10 && !isJammed) {
                if (opticalSensor.isNearObject() && !paperDetected)
                    paperDetected = true;
                if (paperDetected && !opticalSensor.isNearObject())
                    isClear = true;
                else if (!opticalSensor.isNearObject())
                    paperDetected = false;
                
                if (controller.ButtonEUp.pressing()) {
                    rollers.stop();
                    feeder.stop();
                    while (controller.ButtonEUp.pressing()) {
                        printf("Paused 1\n");
                        wait(100, vex::msec);
                    }
                    while (!controller.ButtonEUp.pressing()) {
                        printf("Paused 2\n");
                        wait(100, vex::msec);
                    }
                    while (controller.ButtonEUp.pressing()) {
                        printf("Paused 3\n");
                        wait(100, vex::msec);
                    }
                    rollers.setVelocity(80, vex::velocityUnits::rpm);
                    rollers.spin(vex::directionType::fwd);
                    indexTime = brain.Timer.value();
                }

                if (rollers.isStalling(60, 70)) {
                    rollers.stop();
                    printf("Rollers stalled: %f, %f\n", rollers.velocity(vex::velocityUnits::rpm), rollers.current(vex::percentUnits::pct));
                    isJammed = true;
                    errorCode = 1;
                }

                wait(20, vex::msec);
            }

            if (brain.Timer.value() - indexTime >= 10) {
                isJammed = true;
                errorCode = 2;
            }
        }

        paused = false;
    }

    rollers.stop();
    feeder.stop();
    hardstop1.stop();
    hardstop2.stop();

    displayError(errorCode);
    while (!touchLed.pressing()) {}

    brain.programStop();
}