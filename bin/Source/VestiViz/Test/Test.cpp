#include <iostream>
#include<random>

#include "MultiplyFilter.h"
#include "ConvOutF.h"
#include "LogSIF.h"
#include "ExpDecaySIF.h"
#include "RegDiffSIF.h"
#include "QCompSIF.h"
#include "DynMatMultDIF.h"
#include "LinCombDIF.h"
#include "StatMatMultSIF.h"

#include "DatumMatrix.h"

//#define _DEBUG_LOG_PIPE
#include "VestivizPipeline.h"


using namespace std::chrono_literals;


void Test1() {
    MultiplyProcessor m1(2);
    LogSIF<double, double, Test_IOWrapper> l1("Doubled ");
    ExpDecaySIF<double,double, Test_IOWrapper> e1(1);
    LogSIF<double, double, Test_IOWrapper> l2("Decay ");
    ConvOutF<double, double, Test_IOWrapper> a1({ 0.5f,0.5f });
    LogOF<double, double, Test_IOWrapper> l3("Output ");
    auto input = Test_IOWrapper::Unwrap<TDf>(m1.getInput(0));

    m1.setOutput(l1.getInput(0));
    l1.setOutput(e1.getInput(0));
    e1.setOutput(l2.getInput(0));
    l2.setOutput(a1.getInput(0));
    a1.setOutput(l3.getInput(0));

    m1.startProcessing();
    l1.startProcessing();
    e1.startProcessing();
    l2.startProcessing();
    a1.startProcessing();
    l3.startProcessing();

    for (int i = 0; i < 100; i++) {
        input->addDatum(TDf{ (double)1,0.01 * (double)i });
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
    RegDiffSIF<double,double, Test_IOWrapper> rd1(16);
    LogSIF<double, double, Test_IOWrapper> l1("Accel: ");
  
    auto input = Test_IOWrapper::Unwrap<TDf>(rd1.getInput(0));

    rd1.setOutput(l1.getInput(0));
   

    rd1.startProcessing();
    l1.startProcessing();
   
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> rng(-0.005f,0.005f);
    for (int i = 0; i < 100; i++) {
        input->addDatum(TDf{ (double)i*i*0.0004 + rng(gen),0.01 * (double)i });
       // std::this_thread::sleep_for(15ms);
    }
    rd1.stopProcessing();
    l1.stopProcessing();

    std::cout << "End";
}

void Test3() {
    Vec3Datum<double> myVec( 2.0f,3.0f,4.0f );

    Vec3Datum<double> compScale1(2.0f, 3.0f, 4.0f);
    std::cout << Datacomp<double, Vec3Datum<double>>::qComp(myVec, compScale1) << std::endl;
    Datacomp<double, Vec3Datum<double>>::qCompEq(myVec, compScale1);
    std::cout << myVec << std::endl;

    std::cout << "End";
}
void Test4() {
    QCompSIF<double,double, Test_IOWrapper> rd1(0.5f);
    LogSIF<double, double, Test_IOWrapper> l1("Compress: ");

    auto input = Test_IOWrapper::Unwrap<TDf>(rd1.getInput(0));

    rd1.setOutput(l1.getInput(0));


    rd1.startProcessing();
    l1.startProcessing();

    for (int i = 0; i < 100; i++) {
        input->addDatum(TDf{ (double)i*0.01f ,0.01f * (double)i });
        //std::this_thread::sleep_for(15ms);
    }
    rd1.stopProcessing();
    l1.stopProcessing();

    std::cout << "End";
}

void Test5() {
    StatMatMultSIF<double,double, Test_IOWrapper,2,3> s1(DatumMatrix<double,2,3>(1.0f,2.0f,3.0f,
                                                              4.0f, 5.0f, 6.0f));
    LogSIF<double,DatumArr<double,double,2>, Test_IOWrapper> l1("Stat ");
    DynMatMultDIF<double, double, Test_IOWrapper, 1, 2> d1;
    LogSIF<double, DatumArr<double, double, 1>, Test_IOWrapper> l2("Dyn ");
    LinCombDIF<double, DatumArr<double, double, 1>, Test_IOWrapper> c1(2.0f,1.0f);
    LogSIF<double, DatumArr<double, double, 1>, Test_IOWrapper> l3("Comb ");

    auto input = Test_IOWrapper::Unwrap<TimedDatum<double, DatumArr<double, double, 3>>>(s1.getInput(0));
    auto matInput = Test_IOWrapper::Unwrap<TimedDatum<double, DatumMatrix<double, 1, 2>>>(d1.getInput(1));
    auto inputConst = Test_IOWrapper::Unwrap<TimedDatum<double, DatumArr<double, double, 1>>>(c1.getInput(1));

    s1.setOutput(l1.getInput(0));
    l1.setOutput(d1.getInput(0));
    d1.setOutput(l2.getInput(0));
    l2.setOutput(c1.getInput(0));
    c1.setOutput(l3.getInput(0));

    s1.startProcessing();
    l1.startProcessing();
    d1.startProcessing();
    l2.startProcessing();
    c1.startProcessing();
    l3.startProcessing();

    for (int i = 0; i < 100; i++) {
        input->addDatum(TimedDatum<double, DatumArr<double, double, 3>>
            (0.01f * (double)i, DatumArr<double, double, 3>((double)i, (double)i, (double)i)));
        matInput->addDatum(TimedDatum<double, DatumMatrix<double, 1, 2>>
            (0.01f * (double)i, DatumMatrix<double, 1, 2>((double)i, (double)i)));
        inputConst->addDatum(TimedDatum<double, DatumArr<double, double, 1>>
            (0.01f * (double)i, DatumArr<double, double, 1>((double)i)));
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

void Test6() {
    auto vp = VestivizPipeline<double>();
/*
    vp.init();

    vp.startPipeline();

    for (int i = 0; i < 100; i++) {
        vp.addDatum((double)i, DatumMatrix<double, 3, 3>(1.0f,0.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,0.0f,1.0f), DatumArr<double, double, 3>((double)(i*i)*0.0f, 2.0f, 3.0f));
        auto val = vp.getDatum();
        std::cout << val.datum<<" t: "<< val.t<< std::endl;
        //std::this_thread::sleep_for(30ms);
    }
    */
    vp.stopPipeline();
}

int main()
{
   /* Test1();
    Test2();
    Test3();
    Test4();
    Test5();*/
    Test6();

   /* DatumMatrix<double, 3, 2> mat = DatumMatrix<double, 3, 2>(1.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f);
    std::cout << mat <<std::endl;
    DatumArr<double,double, 2> vec = DatumArr<double,double, 2>(1.0f, 2.0f);
    std::cout << vec << std::endl;
    std::cout << mat.applyTo(vec) << std::endl;
    */
}