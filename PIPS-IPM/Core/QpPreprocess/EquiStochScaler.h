/*
 * EquiStochScaler.h
 *
 *  Created on: 20.12.2017
 *      Author: bzfrehfe
 */

#ifndef PIPS_IPM_CORE_QPPREPROCESS_EQUISTOCHSCALER_H_
#define PIPS_IPM_CORE_QPPREPROCESS_EQUISTOCHSCALER_H_

#include "../QpPreprocess/StochScaler.h"

class Data;


/**  * @defgroup QpPreprocess
 *
 * QP scaler
 * @{
 */

/**
 * Derived class for QP scalers.
 */
class EquiStochScaler : public StochScaler
{
protected:
  void doObjScaling() override;

public:
  EquiStochScaler(Data * prob, bool bitshifting = true);
  virtual ~EquiStochScaler();

  /** scale */
  void scale() override;
};

//@}


#endif /* PIPS_IPM_CORE_QPPREPROCESS_EQUISTOCHSCALER_H_ */
