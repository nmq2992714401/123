#include <iostream>
#include "structdef.h"
int main(){
    std::cout << "SimulationStep=" << sizeof(SimulationStep) << std::endl;
    std::cout << "LineSpectrumConfig=" << sizeof(LineSpectrumConfig) << std::endl;
    std::cout << "MotionState=" << sizeof(MotionState) << std::endl;
    std::cout << "SonarState=" << sizeof(SonarState) << std::endl;
    std::cout << "DetectionResult=" << sizeof(DetectionResult) << std::endl;
    std::cout << "TargetMotionConfig=" << sizeof(TargetMotionConfig) << std::endl;
    return 0;
}
