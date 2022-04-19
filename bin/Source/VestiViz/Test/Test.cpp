#include <iostream>

#include "MultiplyFilter.h"
#include "AverageFilter.h"
#include "LoggerFilter.h"
#include "ExpDecayFilter.h"

using namespace std::chrono_literals;
int main()
{
    MultiplyProcessor m1(2);
    LogSIF l1("Doubled ");
    ExpSIF e1(1);
    LogSIF l2("Decay ");
    AverageOutputProcessor a1;
    LogOF l3("Output ");
    auto input = m1.getInput();

    m1.setOutput(l1.getInput());
    l1.setOutput(e1.getInput());
    e1.setOutput(l2.getInput());
    l2 .setOutput(a1.getInput());
    a1.setOutput(l3.getInput());

    m1.startProcessing();
    l1.startProcessing();
    e1.startProcessing();
    l2.startProcessing();
    a1.startProcessing();
    l3.startProcessing();

    for (int i = 0; i < 1000; i++) {
        input->addDatum(TDf{(float)i,0.001f*(float)i});
        std::this_thread::sleep_for(15ms);
    }
    m1.stopProcessing();
    l1.stopProcessing();
    e1.stopProcessing();
    l2.stopProcessing();
    a1.stopProcessing();
    l3.stopProcessing();

    std::cout << "End";

}
