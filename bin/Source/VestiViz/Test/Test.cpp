#include <iostream>
#include<random>

#include "MultiplyFilter.h"
#include "AverageFilter.h"
#include "LoggerFilter.h"
#include "ExpDecayFilter.h"
#include "RegDiffFilter.h"

using namespace std::chrono_literals;


void Test1() {
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
    l2.setOutput(a1.getInput());
    a1.setOutput(l3.getInput());

    m1.startProcessing();
    l1.startProcessing();
    e1.startProcessing();
    l2.startProcessing();
    a1.startProcessing();
    l3.startProcessing();

    for (int i = 0; i < 100; i++) {
        input->addDatum(TDf{ (float)1,0.01f * (float)i });
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


void Test2() {
    RegDiffSIF rd1(32);
    LogSIF l1("Accel: ");
  
    auto input = rd1.getInput();

    rd1.setOutput(l1.getInput());
   

    rd1.startProcessing();
    l1.startProcessing();
   
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> rng(-0.005f,0.005f);
    for (int i = 0; i < 100; i++) {
        input->addDatum(TDf{ (float)i*i*0.0004f + rng(gen),0.01f * (float)i });
        std::this_thread::sleep_for(15ms);
    }
    rd1.stopProcessing();
    l1.stopProcessing();

    std::cout << "End";
}

int main()
{
    Test1();
    Test2();

}