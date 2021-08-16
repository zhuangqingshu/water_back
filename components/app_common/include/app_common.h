/**
  ******************************************************************************
  * @file    
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */ 
#ifndef __APP_COMMON_H_
#define __APP_COMMON_H_

/* Includes ------------------------------------------------------------------*/
#include "esp_log.h"

/* Public typedef ------------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/
#define APP_VERSION (102)

#define APP_TIME_TO_MS(x) ((x)/portTICK_RATE_MS)

#define APP_MIN_REPORT_INTERVAL_TIME (2)
#define APP_MAX_REPORT_INTERVAL_TIME (10*60)

/* Public macro --------------------------------------------------------------*/
#define APP_ERROR_CHECK(a, str, ret_val) \
    if (!(a)) { \
        ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret_val); \
    }
	

/* Macros take the buffer pointer and return the data */
#define ZNC_RTN_U64( BUFFER, i )  ( ( ( uint64_t ) ( BUFFER )[ i ]  <<  56) |\
    ( ( uint64_t ) ( BUFFER )[ i + 1 ]  << 48) |\
    ( ( uint64_t ) ( BUFFER )[ i + 2 ]  << 40) |\
    ( ( uint64_t ) ( BUFFER )[ i + 3 ]  << 32) |\
    ( ( uint64_t ) ( BUFFER )[ i + 4 ]  << 24) |\
    ( ( uint64_t ) ( BUFFER )[ i + 5 ]  << 16) |\
    ( ( uint64_t ) ( BUFFER )[ i + 6 ]  << 8) |\
    ( ( uint64_t ) ( BUFFER )[ i + 7 ] & 0xFF))\

#define ZNC_RTN_U32( BUFFER, i ) ( ( ( uint32_t ) ( BUFFER )[ i ] << 24) |\
    ( ( uint32_t ) ( BUFFER )[ i + 1 ]  << 16) |\
    ( ( uint32_t ) ( BUFFER )[ i + 2 ]  << 8) |\
    ( ( uint32_t ) ( BUFFER )[ i + 3 ] & 0xFF))\

#define ZNC_RTN_U16( BUFFER, i ) ( ( ( uint16_t ) (BUFFER)[ i ] << 8) |\
    ( ( uint16_t ) (BUFFER)[ i + 1 ] & 0xFF))\

/* Macros take the value and put it into a buffer */
#define ZNC_BUF_U8_UPD( BUFFER, U8VALUE, LEN)    (  ( *( (uint8_t*)( ( BUFFER ) ) ) = ( ( ( ( uint8_t ) ( U8VALUE ) ) & 0xFF ) ) ) ,\
     ( ( LEN ) += sizeof( uint8_t ) ) )

#define ZNC_BUF_U64_UPD( BUFFER, U64VALUE, LEN)    ( ( *(uint8_t*) ( BUFFER ) = (uint8_t) ( ( ( ( uint64_t ) ( U64VALUE ) ) >> 56) & 0xFF)),\
    ( *(uint8_t*) ( ( BUFFER ) + 1) = (uint8_t) ( ( ( ( uint64_t ) ( U64VALUE ) ) >> 48) & 0xFF ) ),\
    ( *(uint8_t*) ( ( BUFFER ) + 2) = (uint8_t) ( ( ( ( uint64_t ) ( U64VALUE ) ) >> 40) & 0xFF ) ),\
    ( *(uint8_t*) ( ( BUFFER ) + 3) = (uint8_t) ( ( ( ( uint64_t ) ( U64VALUE ) ) >> 32) & 0xFF ) ),\
    ( *(uint8_t*) ( ( BUFFER ) + 4) = (uint8_t) ( ( ( ( uint64_t ) ( U64VALUE ) ) >> 24) & 0xFF ) ),\
    ( *(uint8_t*) ( ( BUFFER ) + 5) = (uint8_t) ( ( ( ( uint64_t ) ( U64VALUE ) ) >> 16) & 0xFF ) ),\
    ( *(uint8_t*) ( ( BUFFER ) + 6) = (uint8_t) ( ( ( ( uint64_t ) ( U64VALUE ) ) >>  8) & 0xFF ) ),\
    ( *(uint8_t*) ( ( BUFFER ) + 7) = (uint8_t) ( ( ( ( uint64_t ) ( U64VALUE ) ) & 0xFF ) ) ),\
    ( ( LEN ) += sizeof( uint64_t ) ) )

#define ZNC_BUF_U32_UPD( BUFFER, U32VALUE, LEN )     ( ( *( uint8_t* )( BUFFER )   =  ( uint8_t ) ( ( ( ( uint32_t ) ( U32VALUE ) ) >> 24 ) & 0xFF ) ),\
    ( *( uint8_t* )( ( BUFFER ) + 1) =  ( uint8_t ) ( ( ( ( uint32_t ) ( U32VALUE ) ) >> 16 ) & 0xFF ) ),\
    ( *( uint8_t* )( ( BUFFER ) + 2) =  ( uint8_t ) ( ( ( ( uint32_t ) ( U32VALUE ) ) >> 8 ) & 0xFF ) ),\
    ( *( uint8_t* )( ( BUFFER ) + 3) =  ( uint8_t ) ( ( ( ( uint32_t ) ( U32VALUE ) ) & 0xFF ) ) ) ,\
    ( ( LEN ) += sizeof ( uint32_t ) ) )


#define ZNC_BUF_U16_UPD( BUFFER, U16VALUE, LEN )     ( ( *( uint8_t* )( BUFFER )   =  ( uint8_t )  ( ( ( ( uint16_t)( U16VALUE ) ) >> 8 ) & 0xFF ) ),\
    ( *( uint8_t* ) ( ( BUFFER ) + 1 )  =  ( uint8_t )  ( ( ( ( uint16_t )( U16VALUE ) ) ) & 0xFF ) ) ,\
    ( ( LEN ) += sizeof( uint16_t ) ) )


/* Macros take buffer and return data and the next offset of within the buffer */
#define ZNC_RTN_U16_OFFSET(BUFFER, i, OFFSET )   ( ZNC_RTN_U16 (BUFFER, i) );\
( ( OFFSET ) += sizeof (uint16_t) )

#define ZNC_RTN_U32_OFFSET(BUFFER, i, OFFSET )   (  ZNC_RTN_U32 (BUFFER, i) );\
( ( OFFSET ) += sizeof (uint32_t) )

#define ZNC_RTN_U64_OFFSET(BUFFER, i, OFFSET )  (  ZNC_RTN_U64 (BUFFER, i) );\
( ( OFFSET ) += sizeof (uint64_t) )

/* Public variables ----------------------------------------------------------*/

//extern struct station_config s_staconf;

/* Public function prototypes ------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/


#endif /* __APP_COMMON_H_ */
/********************************* END OF FILE *********************************/


