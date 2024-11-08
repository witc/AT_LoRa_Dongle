/****************************************************************/
/*                        I N C L U D E                         */
/****************************************************************/
#include "main.h"
#include "portSTM32L071xx.h"

#if MCC_USE_LOG_LIB
#define LOG_TAG "[ATCM-G0]"
#define LOG_LEVEL MCU_PLAT_LOG_LEVEL
#include "Log.h"
#else
#define LOG_FATAL(...) 		do{} while(0)
#define LOG_ERROR(...)		do{} while(0)
#define LOG_WARNING(...)	do{} while(0)
#define LOG_INFO(...)		do{} while(0)
#define LOG_DEBUG(...)		do{} while(0)
#define LOG_VERBOOSE(...)	do{} while(0)
#endif

/****************************************************************/
/*      S T A T I C   F U N C T I O N   P R O T O T Y P E       */
/****************************************************************/
static HAL_StatusTypeDef SP_StartRecToIdle_DMA(UART_HandleTypeDef *huart, void *pData, uint16_t Size);

/****************************************************************/
/*            F U N C T I O N   D E F I N I T I O N             */
/****************************************************************/

/**
 * @brief 
 * 
 * @param sp_ctx 
 * @return true 
 * @return false 
 */
bool SP_PlatformInit(SP_Context_t *sp_ctx)
{
	if (sp_ctx == NULL || sp_ctx->phuart == NULL)
	{
		LOG_ERROR("Invalid argument.");
		return false;
	}
	
	return SP_StartRecToIdle_DMA(sp_ctx->phuart, sp_ctx->rxStorage.raw_data, sp_ctx->rxStorage.size);
}


/**
 * @brief 
 * 
 * @param huart 
 * @param pData 
 * @param Size 
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef SP_StartRecToIdle_DMA(UART_HandleTypeDef *huart, void *pData, uint16_t Size)
{
	if (HAL_UARTEx_ReceiveToIdle_DMA(huart, pData, Size) == HAL_OK)
	{
		__HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
		return HAL_OK;
	}
	else
	{
		__HAL_UART_CLEAR_FLAG(huart, 0xFFFFFFFF);
		HAL_UART_AbortReceive(huart);
		if (HAL_UARTEx_ReceiveToIdle_DMA(huart, pData, Size) == HAL_OK)
		{
			__HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
			return HAL_OK;
		}
		else
		{
			return HAL_ERROR;
		}
	}
}


/**
 * @brief 
 * 
 * @param sp_ctx 
 * @param size 
 * @return true 
 * @return false 
 */
bool SP_RxComplete(SP_Context_t *sp_ctx, uint16_t size)
{

	HAL_StatusTypeDef ret = SP_StartRecToIdle_DMA(sp_ctx->phuart, sp_ctx->rxStorage.raw_data, sp_ctx->rxStorage.size);
	return (ret == HAL_OK) ? true : false;
}


/**
 * @brief 
 * 
 * @param sp_ctx 
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef SP_HandleUARTError(SP_Context_t *sp_ctx)
{
	if (sp_ctx == NULL || sp_ctx->phuart == NULL)
	{
		LOG_ERROR("Invalid argument in %s.", __func__);
		return HAL_ERROR;
	}

	if (HAL_UART_GetError(sp_ctx->phuart) != HAL_UART_ERROR_NONE)
	{
		return SP_StartRecToIdle_DMA(sp_ctx->phuart, sp_ctx->rxStorage.raw_data, sp_ctx->rxStorage.size);
	}

	return HAL_ERROR;
}