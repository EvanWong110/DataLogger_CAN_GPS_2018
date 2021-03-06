/*
 * can_receiver.c
 *
 *  Created on: 03.06.2017
 *      Author: Michal Kowalik
 */

#include "user/can_receiver.h"
#include "main.h"

//< ----- Private functions/IRQ Callbacks prototypes ----- >//

CANReceiver_Status_TypeDef	CANReceiver_RxCallback(volatile CANReceiver_TypeDef* pSelf, CANData_TypeDef* pData);
void						CANReceiver_RxCallbackWrapper(CANData_TypeDef* pData, void* pVoidSelf);

CANReceiver_Status_TypeDef	CANReceiver_ErrorCallback(volatile CANReceiver_TypeDef* pSelf, CANErrorCode_TypeDef errorcode);
void						CANReceiver_ErrorCallbackWrapper(CANErrorCode_TypeDef errorcode, void* pVoidSelf);

//< ----- Public functions ----- >//

CANReceiver_Status_TypeDef CANReceiver_init(volatile CANReceiver_TypeDef* pSelf, Config_TypeDef* pConfig, volatile CANTransceiverDriver_TypeDef* pCanTransceiverHandler, volatile MSTimerDriver_TypeDef* pMsTimerDriverHandler){

	if ((pSelf == NULL) || (pConfig == NULL) || (pCanTransceiverHandler == NULL) || (pMsTimerDriverHandler == NULL)){
		return CANReceiver_Status_NullPointerError;
	}

	pSelf->state					= CANReceiver_State_NotInitialised;
	pSelf->pConfig					= pConfig;
	pSelf->pCanTransceiverHandler	= pCanTransceiverHandler;
	pSelf->pMsTimerDriverHandler	= pMsTimerDriverHandler;

	if (FIFOQueue_init(&(pSelf->framesFIFO), pSelf->aReceiverQueueBuffer, sizeof(CANData_TypeDef), CAN_MSG_QUEUE_SIZE) != FIFO_Status_OK){
		return CANReceiver_Status_FIFOError;
	}

	if (FIFOQueue_init(&(pSelf->canErrorsFIFO), pSelf->aReceiverCANErrorsQueueBuffer, sizeof(CANErrorData_TypeDef), CAN_ERROR_QUEUE_SIZE) != FIFO_Status_OK){
		return CANReceiver_Status_FIFOError;
	}

	for (uint16_t i=0; i<CAN_MSG_QUEUE_SIZE; i++){
		pSelf->aReceiverQueueBuffer[i] = (CANData_TypeDef){0};
	}

	for (uint16_t i=0; i<CAN_ERROR_QUEUE_SIZE; i++){
		pSelf->aReceiverCANErrorsQueueBuffer[i] = (CANErrorData_TypeDef){0};
	}

	uint16_t aFilterIDsTab[pConfig->numOfFrames];

	for (uint16_t i=0; i<pConfig->numOfFrames; i++){
		aFilterIDsTab[i] = pConfig->canFrames[i].ID;
	}

	if (CANTransceiverDriver_configFiltering(pSelf->pCanTransceiverHandler, aFilterIDsTab, pConfig->numOfFrames) != CANTransceiverDriver_Status_OK){
		return CANReceiver_Status_CANTransceiverDriverError;
	}

	if (CANTransceiverDriver_registerReceiveCallbackToCall(pSelf->pCanTransceiverHandler, CANReceiver_RxCallbackWrapper, (void*) pSelf) != CANTransceiverDriver_Status_OK){
		return CANReceiver_Status_CANTransceiverDriverError;
	}

	if (CANTransceiverDriver_registerErrorCallbackToCall(pSelf->pCanTransceiverHandler, CANReceiver_ErrorCallbackWrapper, (void*) pSelf) != CANTransceiverDriver_Status_OK){
		return CANReceiver_Status_CANTransceiverDriverError;
	}

	pSelf->state = CANReceiver_State_Initialised;

	return CANReceiver_Status_OK;

}

CANReceiver_Status_TypeDef CANReceiver_start(volatile CANReceiver_TypeDef* pSelf){

	if (pSelf == NULL){
		return CANReceiver_Status_NullPointerError;
	}

	if (pSelf->state == CANReceiver_State_NotInitialised){
		return CANReceiver_Status_NotInitialisedError;
	}

	if (pSelf->state == CANReceiver_State_Running){
		return CANReceiver_Status_AlreadyRunningError;
	}

	if (CANTransceiverDriver_start(pSelf->pCanTransceiverHandler) != CANTransceiverDriver_Status_OK){
		return CANReceiver_Status_CANTransceiverDriverError;
	}

	pSelf->state = CANReceiver_State_Running;

	return CANReceiver_Status_OK;
}

CANReceiver_Status_TypeDef CANReceiver_pullLastFrame(volatile CANReceiver_TypeDef* pSelf, CANData_TypeDef* pRetMsg){

	if ((pSelf == NULL) || (pRetMsg == NULL)){
		return CANReceiver_Status_NullPointerError;
	}

	if (pSelf->state != CANReceiver_State_Running){
		return CANReceiver_Status_NotRunningError;
	}

	FIFO_Status_TypeDef fifoStatus = FIFOQueue_dequeue(&(pSelf->framesFIFO), pRetMsg);

	switch(fifoStatus){
		case FIFO_Status_OK:
			return CANReceiver_Status_OK;
		case FIFO_Status_Empty:
			return CANReceiver_Status_Empty;
		case FIFO_Status_Full:
			return CANReceiver_Status_FullFIFOError;
		case FIFO_Status_DequeueInProgressError:
		case FIFO_Status_UnInitializedError:
		case FIFO_Status_Error:
		default:
			return CANReceiver_Status_FIFOError;
	}

	return CANReceiver_Status_OK;
}

CANReceiver_Status_TypeDef CANReceiver_pullLastCANBusError(volatile CANReceiver_TypeDef* pSelf, CANErrorData_TypeDef* pRetErrorData){

	if ((pSelf == NULL) || (pRetErrorData == NULL)){
		return CANReceiver_Status_NullPointerError;
	}

	if (pSelf->state != CANReceiver_State_Running){
		return CANReceiver_Status_NotRunningError;
	}

	FIFO_Status_TypeDef fifoStatus = FIFOQueue_dequeue(&(pSelf->canErrorsFIFO), pRetErrorData);

	switch(fifoStatus){
		case FIFO_Status_OK:
			return CANReceiver_Status_OK;
		case FIFO_Status_Empty:
			return CANReceiver_Status_Empty;
		case FIFO_Status_Full:
			return CANReceiver_Status_FullFIFOError;
		case FIFO_Status_DequeueInProgressError:
		case FIFO_Status_UnInitializedError:
		case FIFO_Status_Error:
		default:
			return CANReceiver_Status_FIFOError;
	}

	return CANReceiver_Status_OK;
}

CANReceiver_Status_TypeDef CANReceiver_clear(volatile CANReceiver_TypeDef* pSelf){

	if (pSelf == NULL){
		return CANReceiver_Status_NullPointerError;
	}

	if (pSelf->state == CANReceiver_State_NotInitialised){
		return CANReceiver_Status_NotInitialisedError;
	}

	if (FIFOQueue_clear(&(pSelf->framesFIFO)) != FIFO_Status_OK){
		return CANReceiver_Status_FIFOError;
	}

	if (FIFOQueue_clear(&(pSelf->canErrorsFIFO)) != FIFO_Status_OK){
		return CANReceiver_Status_FIFOError;
	}

	return CANReceiver_Status_OK;
}

//< ----- Callback functions ----- >//

CANReceiver_Status_TypeDef CANReceiver_RxCallback(volatile CANReceiver_TypeDef* pSelf, CANData_TypeDef* pData){

	if (MSTimerDriver_getMSTime(pSelf->pMsTimerDriverHandler, &pData->msTimestamp) != MSTimerDriver_Status_OK){ //TODO trzeba tu wykorzystac ten czas z CANa
		return CANReceiver_Status_MSTimerError;
	}

	FIFO_Status_TypeDef fifoStatus = FIFOQueue_enqueue(&(pSelf->framesFIFO), pData);
	if (fifoStatus == FIFO_Status_OK){
		return CANReceiver_Status_OK;
	} else if (fifoStatus == FIFO_Status_Full){
		return CANReceiver_Status_FullFIFOError;
	} else {
		return CANReceiver_Status_FIFOError;
	}
}

void CANReceiver_RxCallbackWrapper(CANData_TypeDef* pData, void* pVoidSelf){
	CANReceiver_Status_TypeDef ret = CANReceiver_Status_OK;
	if((ret = CANReceiver_RxCallback((CANReceiver_TypeDef*) pVoidSelf, pData)) != CANReceiver_Status_OK){
		Error_Handler();
	}

}

CANReceiver_Status_TypeDef CANReceiver_ErrorCallback(volatile CANReceiver_TypeDef* pSelf, CANErrorCode_TypeDef errorcode){

	if (errorcode != CANErrorCode_None){

		CANErrorData_TypeDef errData = {
				.errorCode = errorcode
		};

		if (MSTimerDriver_getMSTime(pSelf->pMsTimerDriverHandler, &(errData.msTimestamp)) != MSTimerDriver_Status_OK){ //TODO trzeba tu wykorzystac ten czas z CANa
			return CANReceiver_Status_MSTimerError;
		}

		FIFO_Status_TypeDef fifoStatus = FIFOQueue_enqueue(&(pSelf->canErrorsFIFO), &errData);
		if ((fifoStatus == FIFO_Status_OK) || (fifoStatus == FIFO_Status_Full)){ //< Ignoring FIFO_Status_Full error on purpose.
			return CANReceiver_Status_OK;
		} else {
			return CANReceiver_Status_FIFOError;
		}
	}

	return CANReceiver_Status_OK;

}

void CANReceiver_ErrorCallbackWrapper(CANErrorCode_TypeDef errorcode, void* pVoidSelf){

	if (CANReceiver_ErrorCallback((CANReceiver_TypeDef*) pVoidSelf, errorcode) != CANReceiver_Status_OK){
		Error_Handler();
	}

}
