#pragma once
#ifndef _PIB_WRAPPER_H_
#define _PIB_WRAPPER_H_

#include"TemplateInterchangeWrapper.h"
#include "PostboxInputBase.h"
#include "TimedDatum.h"
#include "DatumMatrix.h"

using PIB_Wrapper = TemplateInterchangeWrapper<
	PostboxInputBase, 
	TimedDatum<double, DatumArr<double, double, 6>>,
	TimedDatum<double, DatumArr<double, double, 3>>,
	TimedDatum<double, DatumMatrix<double, 3, 3>>,
	TimedDatum<double, DatumArr<double, double, 8>>,
	TimedDatum<double, DatumArr<double, double, 4>> > ;
#endif