//
// Created by kyle on 11/10/2025.
//
#include "ILabJacks.h"
#include "LabJacksController.h"


LabJacksController::LabJacksController() : error(0), handle(0), volt0(0.0), volt1(0.0),
    volt2(0.0), volt3(0.0) {
    // Open the LabJack T7 device
}
[[nodiscard]] double LabJacksController::getVoltageAINO() const {
    return volt0;
}
[[nodiscard]] double LabJacksController::getVoltageAINOne() const {
    return volt1;
}
[[nodiscard]] double LabJacksController::getVoltageAINTwo() const {
    return volt2;
}
[[nodiscard]] double LabJacksController::getVoltageAINThree() const {
    return volt3;
}
//goes up to 7 with T7 I believe. based off docs
LabJacksController::~LabJacksController() {
    LJM_Close(handle);
}