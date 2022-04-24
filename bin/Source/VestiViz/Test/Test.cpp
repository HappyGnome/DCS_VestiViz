#include <iostream>
#include<random>

#include "MultiplyFilter.h"
#include "ConvOutF.h"
#include "LoggerFilter.h"
#include "ExpDecaySIF.h"
#include "RegDiffSIF.h"
#include "QCompSIF.h"
#include "DynMatMultDIF.h"
#include "LinCombDIF.h"
#include "StatMatMultSIF.h"

#include "DatumMatrix.h"

using namespace std::chrono_literals;


void Test1() {
    MultiplyProcessor m1(2);
    LogSIF<float, float> l1("Doubled ");
    ExpDecaySIF<float,float> e1(1);
    LogSIF<float, float> l2("Decay ");
    ConvOutF<float, float> a1({ 0.5f,0.5f });
    LogOF<float, float> l3("Output ");
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
        //std::this_thread::sleep_for(15ms);
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
    RegDiffSIF<float,float> rd1(16);
    LogSIF<float, float> l1("Accel: ");
  
    auto input = rd1.getInput();

    rd1.setOutput(l1.getInput());
   

    rd1.startProcessing();
    l1.startProcessing();
   
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> rng(-0.005f,0.005f);
    for (int i = 0; i < 100; i++) {
        input->addDatum(TDf{ (float)i*i*0.0004f + rng(gen),0.01f * (float)i });
       // std::this_thread::sleep_for(15ms);
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
void Test4() {
    QCompSIF<float,float> rd1(0.5f);
    LogSIF<float, float> l1("Compress: ");

    auto input = rd1.getInput();

    rd1.setOutput(l1.getInput());


    rd1.startProcessing();
    l1.startProcessing();

    for (int i = 0; i < 100; i++) {
        input->addDatum(TDf{ (float)i*0.01f ,0.01f * (float)i });
        //std::this_thread::sleep_for(15ms);
    }
    rd1.stopProcessing();
    l1.stopProcessing();

    std::cout << "End";
}

void Test5() {
    StatMatMultSIF<float,float,2,3> s1(DatumMatrix<float,2,3>(1.0f,2.0f,3.0f,
                                                              4.0f, 5.0f, 6.0f));
    LogSIF<float,DatumArr<float,float,2>> l1("Stat ");
    DynMatMultDIF<float, float, 1, 2> d1;
    LogSIF<float, DatumArr<float, float, 1>> l2("Dyn ");
    LinCombDIF<float, DatumArr<float, float, 1>> c1(2.0f,1.0f);
    LogSIF<float, DatumArr<float, float, 1>> l3("Comb ");

    auto input = s1.getInput();
    auto matInput = d1.getInput2();
    auto inputConst = c1.getInput2();

    s1.setOutput(l1.getInput());
    l1.setOutput(d1.getInput1());
    d1.setOutput(l2.getInput());
    l2.setOutput(c1.getInput1());
    c1.setOutput(l3.getInput());

    s1.startProcessing();
    l1.startProcessing();
    d1.startProcessing();
    l2.startProcessing();
    c1.startProcessing();
    l3.startProcessing();

    for (int i = 0; i < 100; i++) {
        input->addDatum(TimedDatum<float, DatumArr<float, float, 3>>{DatumArr<float, float, 3>((float)i, (float)i, (float)i),0.01f * (float)i });
        matInput->addDatum(TimedDatum<float, DatumMatrix<float, 1, 2>>{DatumMatrix<float, 1, 2>((float)i, (float)i), 0.01f * (float)i });
        inputConst->addDatum(TimedDatum<float, DatumArr<float, float, 1>>{DatumArr<float, float, 1>((float)i), 0.01f * (float)i });
        //std::this_thread::sleep_for(15ms);
    }
    s1.stopProcessing();
    l1.stopProcessing();
    d1.stopProcessing();
    l2.stopProcessing();
    c1.stopProcessing();
    l3.stopProcessing();

    std::cout << "End";
}


int main()
{
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();

    DatumMatrix<float, 3, 2> mat = DatumMatrix<float, 3, 2>(1.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f);
    std::cout << mat <<std::endl;
    DatumArr<float,float, 2> vec = DatumArr<float,float, 2>(1.0f, 2.0f);
    std::cout << vec << std::endl;
    std::cout << mat.applyTo(vec) << std::endl;

}