
#include "esp_system.h"
#include "esp_log.h"
#include <math.h>

#define _NTC_R_SERIES         (150000.0f)
#define _NTC_R_NOMINAL        10000.0f
#define _NTC_TEMP_NOMINAL     25.0f
#define _NTC_ADC_MAX          1023.0f //  10bit
#define _NTC_BETA             3950
#define _NTC_ADC_VREF         1.0f // v
#define _NTC_VDD              3.26f // v



static const char *TAG = "ntc_sensor";

//#define ADC_2_V(x) ((x) * _NTC_ADC_VREF / _NTC_ADC_MAX)
#define ADC_2_V(x) ((x) * 0.000917874396135)

static float get_r(uint32_t adcValue)
{
	return _NTC_R_SERIES / ((_NTC_VDD / ADC_2_V(adcValue)) - 1.0f);
}

//#######################################################################################
float ntc_convert2c(uint32_t adcValue)
{
  float rntc = get_r(adcValue);
  ESP_LOGD(TAG, "ADC %d V %d NTC R %d", (int)adcValue, (int)(ADC_2_V(adcValue)*1000), (int)rntc);
  float temp;
  temp = rntc / (float)_NTC_R_NOMINAL; 
  temp = logf(temp);
  temp /= (float)_NTC_BETA;
  temp += 1.0f / ((float)_NTC_TEMP_NOMINAL + 273.15f);
  temp = 1.0f / temp;
  temp -= 273.15f;
  return temp;
}
//#######################################################################################
float ntc_convert2f(uint32_t adcValue)
{
  float rntc = get_r(adcValue);
  float temp;
  temp = rntc / (float)_NTC_R_NOMINAL; 
  temp = logf(temp);
  temp /= (float)_NTC_BETA;
  temp += 1.0f / ((float)_NTC_TEMP_NOMINAL + 273.15f);
  temp = 1.0f / temp;
  temp -= 273.15f;
  return (temp * 9.0f / 5.0f) + 32.f;
}
//#######################################################################################

