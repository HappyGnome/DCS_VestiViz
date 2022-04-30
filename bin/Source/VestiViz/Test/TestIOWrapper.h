#pragma once
#ifndef _TESTIOWRAPPER_H_
#define _TESTIOWRAPPER_H_

#include"TemplateInterchangeWrapper.h"
#include "PostboxInputBase.h"
#include "TimedDatum.h"
#include "DatumMatrix.h"

using Test_IOWrapper = TemplateInterchangeWrapper<
	PostboxInputBase,
	TimedDatum<float, float>,
	TimedDatum<float, DatumArr<float, float, 2>>,
	TimedDatum<float, DatumArr<float, float, 3>>,
	TimedDatum<float, DatumMatrix<float, 1, 2>>,
	TimedDatum<float, DatumArr<float, float, 1>>>;

#endif