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
    LogSIF<float, float, Test_IOWrapper> l1("Doubled ");
    ExpDecaySIF<float,float, Test_IOWrapper> e1(1);
    LogSIF<float, float, Test_IOWrapper> l2("Decay ");
    ConvOutF<float, float, Test_IOWrapper> a1({ 0.5f,0.5f });
    LogOF<float, float, Test_IOWrapper> l3("Output ");
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
    RegDiffSIF<float,float, Test_IOWrapper> rd1(16);
    LogSIF<float, float, Test_IOWrapper> l1("Accel: ");
  
    auto input = Test_IOWrapper::Unwrap<TDf>(rd1.getInput(0));

    rd1.setOutput(l1.getInput(0));
   

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
    QCompSIF<float,float, Test_IOWrapper> rd1(0.5f);
    LogSIF<float, float, Test_IOWrapper> l1("Compress: ");

    auto input = Test_IOWrapper::Unwrap<TDf>(rd1.getInput(0));

    rd1.setOutput(l1.getInput(0));


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
    StatMatMultSIF<float,float, Test_IOWrapper,2,3> s1(DatumMatrix<float,2,3>(1.0f,2.0f,3.0f,
                                                              4.0f, 5.0f, 6.0f));
    LogSIF<float,DatumArr<float,float,2>, Test_IOWrapper> l1("Stat ");
    DynMatMultDIF<float, float, Test_IOWrapper, 1, 2> d1;
    LogSIF<float, DatumArr<float, float, 1>, Test_IOWrapper> l2("Dyn ");
    LinCombDIF<float, DatumArr<float, float, 1>, Test_IOWrapper> c1(2.0f,1.0f);
    LogSIF<float, DatumArr<float, float, 1>, Test_IOWrapper> l3("Comb ");

    auto input = Test_IOWrapper::Unwrap<TimedDatum<float, DatumArr<float, float, 3>>>(s1.getInput(0));
    auto matInput = Test_IOWrapper::Unwrap<TimedDatum<float, DatumMatrix<float, 1, 2>>>(d1.getInput(1));
    auto inputConst = Test_IOWrapper::Unwrap<TimedDatum<float, DatumArr<float, float, 1>>>(c1.getInput(1));

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
        input->addDatum(TimedDatum<float, DatumArr<float, float, 3>>
            (0.01f * (float)i, DatumArr<float, float, 3>((float)i, (float)i, (float)i)));
        matInput->addDatum(TimedDatum<float, DatumMatrix<float, 1, 2>>
            (0.01f * (float)i, DatumMatrix<float, 1, 2>((float)i, (float)i)));
        inputConst->addDatum(TimedDatum<float, DatumArr<float, float, 1>>
            (0.01f * (float)i, DatumArr<float, float, 1>((float)i)));
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
    auto vp = VestivizPipeline<float>();

    vp.init();

    vp.startPipeline();

    for (int i = 0; i < 100; i++) {
        vp.addDatum((float)i, DatumMatrix<float, 3, 3>(1.0f,0.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,0.0f,1.0f), DatumArr<float, float, 3>((float)(i*i)*0.0f, 2.0f, 3.0f));
        auto val = vp.getDatum();
        std::cout << val.datum<<" t: "<< val.t<< std::endl;
        //std::this_thread::sleep_for(30ms);
    }

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

   /* DatumMatrix<float, 3, 2> mat = DatumMatrix<float, 3, 2>(1.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f);
    std::cout << mat <<std::endl;
    DatumArr<float,float, 2> vec = DatumArr<float,float, 2>(1.0f, 2.0f);
    std::cout << vec << std::endl;
    std::cout << mat.applyTo(vec) << std::endl;
    */
}