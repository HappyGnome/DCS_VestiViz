#pragma once
#ifndef _TESTIOWRAPPER_H_
#define _TESTIOWRAPPER_H_

#include"TemplateInterchangeWrapper.h"
#include "PostboxInputBase.h"
#include "TimedDatum.h"
#include "DatumMatrix.h"

using Test_IOWrapper = TemplateInterchangeWrapper<
	PostboxInputBase,
	TimedDatum<double, double>,
	TimedDatum<double, DatumArr<double, double, 2>>,
	TimedDatum<double, DatumArr<double, double, 3>>,
	TimedDatum<double, DatumMatrix<double, 1, 2>>,
	TimedDatum<double, DatumArr<double, double, 1>>>;

#endif