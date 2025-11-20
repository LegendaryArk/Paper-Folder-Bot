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

Feeder feeder(feederMotor, distanceSensor, brain.Timer);
Hardstop hardstop1(hardstop1Motor, 0.02, 0.015, 0.0013, brain.Timer);
Hardstop hardstop2(hardstop2Motor, 0.02, 0.015, 0.0013, brain.Timer);

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
            fold1 = Hardstop::MIN_FOLD_POS_CM;
            fold2 = Hardstop::MIN_FOLD_POS_CM * 2;
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
                else
                    *foldLocs[i / 2] += precision;
            } else if (!btns[i]->pressing()) {
                if (brain.Timer.value() - pressSt[i] < 0.5) {
                    if (i % 2)
                        *foldLocs[i / 2] -= precision;
                    else
                        *foldLocs[i / 2] += precision;
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

        for (int i = 0; i < 2; i++) {
            *foldLocs[i] = Util::clamp(*foldLocs[i], Hardstop::MIN_FOLD_POS_CM + Hardstop::MAX_FOLD_POS_CM * i, Hardstop::MAX_FOLD_POS_CM * (i + 1));
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

	double fold1Location = Hardstop::MIN_FOLD_POS_CM;
    double fold2Location = Hardstop::MIN_FOLD_POS_CM * 2;
    inputFoldLocations(fold1Location, fold2Location);

    brain.Screen.clearLine(3);
    brain.Screen.clearLine(5);

    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Initializing...");

    hardstop1.setTarget(fold1Location);
    hardstop2.setTarget(fold2Location - fold1Location);

    while (!hardstop1.isSettled()) {
        hardstop1.update();
        hardstop2.update();
        wait(20, vex::msec);
    }

    hardstop1.stop();
    hardstop2.stop();

    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Folding...");

    rollers.setVelocity(60, vex::velocityUnits::rpm);
    rollers.spin(vex::directionType::fwd);

    int numPaper = 0;
    brain.Screen.setCursor(4, 1);
    brain.Screen.clearLine();
    brain.Screen.print("Folding... %d", numPaper);

    double indexTime = 0;
    bool isTrayEmpty = false;
    bool isJammed = false;
    while (!isTrayEmpty && !isJammed) {
        isTrayEmpty = !feeder.index(40, 10);

        if (!isTrayEmpty) {
            indexTime = brain.Timer.value();

            numPaper++;
            brain.Screen.setCursor(4, 1);
            brain.Screen.clearLine();
            brain.Screen.print("Folding... %d", numPaper);

            bool isClear = false, paperDetected = false;
            while (!isClear && brain.Timer.value() - indexTime < 20 && !isJammed) {
                if (opticalSensor.isNearObject() && !paperDetected)
                    paperDetected = true;
                if (paperDetected && !opticalSensor.isNearObject())
                    isClear = true;

                if (rollers.isStalling(20, 50)) {
                    rollers.stop();
                    isJammed = true;
                    errorCode = 1;
                }

                wait(20, vex::msec);
            }

            if (brain.Timer.value() - indexTime >= 20) {
                isJammed = true;
                errorCode = 2;
            }
        }
    }

    rollers.stop();
    feeder.stop();
    hardstop1.stop();
    hardstop2.stop();

    displayError(errorCode);
    while (!touchLed.pressing()) {}

    brain.programStop();
}