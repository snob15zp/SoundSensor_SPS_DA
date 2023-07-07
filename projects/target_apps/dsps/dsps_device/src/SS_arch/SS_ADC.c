#include "SS_ADC.h"

static const adc_config_t adc_cfg = {
  .input_mode       = ADC_INPUT_MODE_SINGLE_ENDED,
  .input            = ADC_INPUT_SE_VBAT_HIGH,
  .smpl_time_mult   = 2,
  .continuous       = false,
  .interval_mult    = 0,
  .input_attenuator = ADC_INPUT_ATTN_4X,
  .chopping         = false,
  .oversampling     = 1
};

/*****************************************************************************************
* @brief Измеряем напряжение питания  
 *****************************************************************************************/
uint16_t SSA_getVdd(void)
{
#ifdef __VDD__	
  uint16_t adc_sample;
  adc_sample = adc_get_sample();
  adc_sample = adc_correct_sample(adc_sample);
    // Divider depends on oversampling setting
  return (uint16_t)((3600 * adc_sample) / 2047);
#else	
	return 2800;
#endif	
}

void SSA_init(void)
{
#ifdef __VDD__	
  adc_offset_calibrate(ADC_INPUT_MODE_SINGLE_ENDED);
  adc_init(&adc_cfg); 	
#endif	
}	


