#include <iostream>

#include "MultiplyProcessor.h"
using namespace std::chrono_literals;
int main()
{
    MultiplyProcessor m1;
    MultiplyProcessor m2;
    AverageOutputProcessor a1;
    auto input = m1.getInput();

    m1.scale = 2;
    m2.scale = 0.5;

    m1.setOutput(m2.getInput());
    m2.setOutput(a1.getInput());

    m1.startProcessing();
    m2.startProcessing();
    a1.startProcessing();

    for (int i = 0; i < 10; i++) {
        input->addDatum(float(i));
        std::this_thread::sleep_for(10ms);
    }
    std::cout << "End";
    m1.stopProcessing();
    m2.stopProcessing();
    a1.stopProcessing();
}
