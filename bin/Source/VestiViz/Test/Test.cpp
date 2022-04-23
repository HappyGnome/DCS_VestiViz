#include <iostream>
#include<random>

#include "MultiplyFilter.h"
#include "AverageFilter.h"
#include "LoggerFilter.h"
#include "ExpDecayFilter.h"
#include "RegDiffFilter.h"

#include "DatumMatrix.h"

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
    RegDiffSIF rd1(16);
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

void Test3() {
    Vec3Datum<float> myVec( 2.0f,3.0f,4.0f );

    Vec3Datum<float> compScale1(2.0f, 3.0f, 4.0f);
    std::cout << Datacomp<float, Vec3Datum<float>>::qComp(myVec, compScale1) << std::endl;
    Datacomp<float, Vec3Datum<float>>::qCompEq(myVec, compScale1);
    std::cout << myVec << std::endl;

    std::cout << "End";
}

int main()
{
    Test1();
    Test2();
    Test3();

    DatumMatrix<float, 3, 2> mat = DatumMatrix<float, 3, 2>(1.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f);
    std::cout << mat <<std::endl;
    DatumArr<float,float, 2> vec = DatumArr<float,float, 2>(1.0f, 2.0f);
    std::cout << vec << std::endl;
    std::cout << mat.applyTo(vec) << std::endl;

}