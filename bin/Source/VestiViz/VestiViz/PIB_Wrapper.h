#pragma once
#ifndef _PIB_WRAPPER_H_
#define _PIB_WRAPPER_H_

#include"TemplateInterchangeWrapper.h"
#include "PostboxInputBase.h"
#include "TimedDatum.h"
#include "DatumMatrix.h"

using PIB_Wrapper = TemplateInterchangeWrapper<
	PostboxInputBase, 
	TimedDatum<float, DatumArr<float, float, 6>>,
	TimedDatum<float, DatumArr<float, float, 3>>,
	TimedDatum<float, DatumMatrix<float, 3, 3>>,
	TimedDatum<float, DatumArr<float, float, 8>>>;

#endif