/*
 * FilterC_s19s29_CG1.c
 *
 * Code generation for model "FilterC_s19s29_CG1".
 *
 * Model version              : 1.28
 * Simulink Coder version : 9.6 (R2021b) 14-May-2021
 * C source code generated on : Sat Sep 10 15:19:44 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "MathFast.h"

#include "FilterC_s19s29_CG1.h"
#include "FilterC_s19s29_CG1_private.h"

/* Block states (default storage) */
DW_FilterC_s19s29_CG1_T FilterC_s19s29_CG1_DW;

/* External inputs (root inport signals with default storage) */
ExtU_FilterC_s19s29_CG1_T FilterC_s19s29_CG1_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_FilterC_s19s29_CG1_T FilterC_s19s29_CG1_Y;

/* Real-time model */
static RT_MODEL_FilterC_s19s29_CG1_T FilterC_s19s29_CG1_M_;
RT_MODEL_FilterC_s19s29_CG1_T *const FilterC_s19s29_CG1_M =
  &FilterC_s19s29_CG1_M_;
void mul_wide_s32(int32_T in0, int32_T in1, uint32_T *ptrOutBitsHi, uint32_T
                  *ptrOutBitsLo)
{
  uint32_T absIn0;
  uint32_T absIn1;
  uint32_T in0Hi;
  uint32_T in0Lo;
  uint32_T in1Hi;
  uint32_T productHiLo;
  uint32_T productLoHi;
  absIn0 = in0 < 0 ? ~(uint32_T)in0 + 1U : (uint32_T)in0;
  absIn1 = in1 < 0 ? ~(uint32_T)in1 + 1U : (uint32_T)in1;
  in0Hi = absIn0 >> 16U;
  in0Lo = absIn0 & 65535U;
  in1Hi = absIn1 >> 16U;
  absIn0 = absIn1 & 65535U;
  productHiLo = in0Hi * absIn0;
  productLoHi = in0Lo * in1Hi;
  absIn0 *= in0Lo;
  absIn1 = 0U;
  in0Lo = (productLoHi << 16U) + absIn0;
  if (in0Lo < absIn0) {
    absIn1 = 1U;
  }

  absIn0 = in0Lo;
  in0Lo += productHiLo << 16U;
  if (in0Lo < absIn0) {
    absIn1++;
  }

  absIn0 = (((productLoHi >> 16U) + (productHiLo >> 16U)) + in0Hi * in1Hi) +
    absIn1;
  if ((in0 != 0) && ((in1 != 0) && ((in0 > 0) != (in1 > 0)))) {
    absIn0 = ~absIn0;
    in0Lo = ~in0Lo;
    in0Lo++;
    if (in0Lo == 0U) {
      absIn0++;
    }
  }

  *ptrOutBitsHi = absIn0;
  *ptrOutBitsLo = in0Lo;
}

//int32_T mul_s32_loSR(int32_T a, int32_T b, uint32_T aShift)
//{
//  uint32_T u32_chi;
//  uint32_T u32_clo;
//  mul_wide_s32(a, b, &u32_chi, &u32_clo);
//  u32_clo = u32_chi << (32U - aShift) | u32_clo >> aShift;
//  return (int32_T)u32_clo;
//}

//static inline int32_T mul_u18s29sh(int32_T a, int32_T b, uint32_T aShift) 
//{
//	int32_T ab;
//	uint32_T b_h;
//	uint32_T b_l;
//	if (b>=0)
//	{
//	b_h = ((uint32_T)b) >> 14;
//  b_l = ((uint32_T )b) &	0x3fff;
//	ab=( (b_h * a )>>(4+aShift) ) 	+  ( (b_l * a )>> (18+aShift) );
//	}
//	else
//	{
//		b=-b;
//	b_h = ((uint32_T)b) >> 14;
//  b_l = ((uint32_T )b) &	0x3fff;
//	ab=( (b_h * a )>>(4+aShift) )	+ ( (b_l * a )>> (18+aShift));
//	ab=-ab;	
//	}
//	return ab;
//}




/* Model step function */
void FilterC_s19s29_CG1_step(void)
{
  int32_T rtb_HeadSum1;
  int32_T rtb_HeadSum2;

  /* Sum: '<S1>/HeadSum1' incorporates:
   *  Delay: '<S1>/BodyDelay21'
   *  Inport: '<Root>/Input'
   */
  rtb_HeadSum1 = (FilterC_s19s29_CG1_U.Input >> 1) +
    FilterC_s19s29_CG1_DW.BodyDelay21_DSTATE;

  /* Sum: '<S1>/HeadSum2' incorporates:
   *  Delay: '<S1>/BodyDelay22'
   *  Sum: '<S1>/HeadSum1'
   */
  rtb_HeadSum2 = (FilterC_s19s29_CG1_DW.BodyDelay22_DSTATE >> 2) + rtb_HeadSum1;

  /* Outport: '<Root>/Output' incorporates:
   *  Sum: '<S1>/HeadSum2'
   */
  FilterC_s19s29_CG1_Y.Output = rtb_HeadSum2;

  /* Update for Delay: '<S1>/BodyDelay21' incorporates:
   *  Delay: '<S1>/FootDelay1'
   *  Gain: '<S1>/a(2)(1)'
   *  Inport: '<Root>/Input'
   *  Sum: '<S1>/BodyLSum21'
   *  Sum: '<S1>/BodyRSum21'
   *  Sum: '<S1>/HeadSum1'
   */
  FilterC_s19s29_CG1_DW.BodyDelay21_DSTATE =
    ((FilterC_s19s29_CG1_DW.FootDelay1_DSTATE >> 1) + FilterC_s19s29_CG1_U.Input)
    - (mul_u18s29sh(94095, rtb_HeadSum1, 17U) >> 2);

  /* Update for Delay: '<S1>/BodyDelay22' incorporates:
   *  Delay: '<S1>/FootDelay2'
   *  Gain: '<S1>/a(2)(2)'
   *  Gain: '<S1>/b(2)(2)'
   *  Sum: '<S1>/BodyLSum22'
   *  Sum: '<S1>/BodyRSum22'
   *  Sum: '<S1>/HeadSum1'
   *  Sum: '<S1>/HeadSum2'
   */
  FilterC_s19s29_CG1_DW.BodyDelay22_DSTATE =
    (((FilterC_s19s29_CG1_DW.FootDelay2_DSTATE >> 3) - rtb_HeadSum1) -
     mul_u18s29sh(-130543, rtb_HeadSum2, 17U)) << 3;

  /* Update for Delay: '<S1>/FootDelay1' incorporates:
   *  Gain: '<S1>/a(3)(1)'
   *  Inport: '<Root>/Input'
   *  Sum: '<S1>/FootSum1'
   *  Sum: '<S1>/HeadSum1'
   */
  FilterC_s19s29_CG1_DW.FootDelay1_DSTATE = FilterC_s19s29_CG1_U.Input -
    (mul_u18s29sh(135099, rtb_HeadSum1, 17U) >> 6);

  /* Update for Delay: '<S1>/FootDelay2' incorporates:
   *  Gain: '<S1>/a(3)(2)'
   *  Sum: '<S1>/FootSum2'
   *  Sum: '<S1>/HeadSum1'
   *  Sum: '<S1>/HeadSum2'
   */
  FilterC_s19s29_CG1_DW.FootDelay2_DSTATE = (rtb_HeadSum1 - mul_u18s29sh(4063,
    rtb_HeadSum2, 12U)) << 2;
}

/* Model initialize function */
void FilterC_s19s29_CG1_initialize(void)
{
  /* Registration code */

  /* initialize error status */
  rtmSetErrorStatus(FilterC_s19s29_CG1_M, (NULL));

  /* states (dwork) */
  (void) memset((void *)&FilterC_s19s29_CG1_DW, 0,
                sizeof(DW_FilterC_s19s29_CG1_T));

  /* external inputs */
  FilterC_s19s29_CG1_U.Input = 0;

  /* external outputs */
  FilterC_s19s29_CG1_Y.Output = 0;

  /* InitializeConditions for Delay: '<S1>/BodyDelay21' */
  FilterC_s19s29_CG1_DW.BodyDelay21_DSTATE = 0;

  /* InitializeConditions for Delay: '<S1>/BodyDelay22' */
  FilterC_s19s29_CG1_DW.BodyDelay22_DSTATE = 0;

  /* InitializeConditions for Delay: '<S1>/FootDelay1' */
  FilterC_s19s29_CG1_DW.FootDelay1_DSTATE = 0;

  /* InitializeConditions for Delay: '<S1>/FootDelay2' */
  FilterC_s19s29_CG1_DW.FootDelay2_DSTATE = 0;
}

/* Model terminate function */
void FilterC_s19s29_CG1_terminate(void)
{
  /* (no terminate code required) */
}
