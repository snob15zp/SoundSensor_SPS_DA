#ifndef __SPI_ADC_h__
#define __SPI_ADC_h__

#include "ss_global.h"
#include "user_periph_setup.h"
#include "spi_531.h"

    #define ADC_EN_PORT             GPIO_PORT_0
#ifdef __SoundSensor__		
    #define ADC_EN_PIN              GPIO_PIN_11
#else
    #define ADC_EN_PIN              GPIO_PIN_7
#endif

    #define ADC_CLK_PORT            GPIO_PORT_0
    #define ADC_CLK_PIN             GPIO_PIN_4

    #define ADC_DO_PORT             GPIO_PORT_0
    #define ADC_DO_PIN              GPIO_PIN_0

    #define ADC_DI_PORT             GPIO_PORT_0
    #define ADC_DI_PIN              GPIO_PIN_3


#define SPI_ADC_GPIO_MAP   ((uint32_t)ADC_CLK_PIN | ((uint32_t)ADC_CLK_PORT << 4) | \
                        ((uint32_t)ADC_EN_PIN << 8) | ((uint32_t)ADC_EN_PORT << 12) | \
                        ((uint32_t)ADC_DO_PIN << 16) | ((uint32_t)ADC_DO_PORT << 20) | \
                        ((uint32_t)ADC_DI_PIN << 24) | ((uint32_t)ADC_DI_PORT << 28))

typedef union {
    int32_t data_u32;
	  uint16_t	u16[2];
	  uint8_t	masByte[4];
} uni_int32_t;

extern volatile bool SA_flashbit;

extern volatile uni_int32_t SA_in;
extern uni_int32_t SA_out;

extern void timer2_init(void);
extern void SA_SPI_init(void);
extern void intinit(void);

extern void SPI_ADC_init(void);
extern void SPI_ADC_deinit(void);
//extern void SPITreeByts (void);

#if	(D_ADCMODE==3)
extern void ADC_IRQ(void);
#endif

#endif
