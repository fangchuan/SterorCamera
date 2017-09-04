#include "TrackerLightsController.h"

#include "FlyCapture2.h"

#define  GPIO_0_PIN           0
#define  GPIO_1_PIN			  1
#define  GPIO_2_PIN           2
#define  GPIO_3_PIN           3

#define  DIRECTION_OUTPUT     1
#define  DIRECTION_INPUT      0

#define  GPIO_0_CTL_ADDRESS   0x1110  //Pin 0
#define  GPIO_1_CTL_ADDRESS   0x1120  //Pin 1
#define  GPIO_2_CTL_ADDRESS   0x1130  //Pin 2  
#define  GPIO_3_CTL_ADDRESS   0x1140  //Pin 3

#define  GPIO_PIN_STA_ADDRESS 0x1100

#define  GPIO_HIGH_VOLTAGE    0x80080001
#define  GPIO_LOW_VOLTAGE     0x80080000

TrackerLightsController::TrackerLightsController()
	: m_LeftCamera(NULL),
	m_RightCamera(NULL)
{

}

TrackerLightsController::~TrackerLightsController()
{

}

void TrackerLightsController::setLeftCamera(Camera *camera)
{
	m_LeftCamera = camera;
	m_LeftCamera->SetGPIOPinDirection(GPIO_2_PIN, DIRECTION_OUTPUT);
}

void TrackerLightsController::setRightCamera(Camera *camera)
{
	m_RightCamera = camera;
	m_RightCamera->SetGPIOPinDirection(GPIO_2_PIN, DIRECTION_OUTPUT);
}

void TrackerLightsController::turnOnLeftLight()
{
	if (m_LeftCamera)
	{
		m_LeftCamera->WriteRegister(GPIO_2_CTL_ADDRESS, GPIO_HIGH_VOLTAGE);
	}
}

void TrackerLightsController::turnOnRightLight()
{
	if (m_RightCamera)
	{
		m_RightCamera->WriteRegister(GPIO_2_CTL_ADDRESS, GPIO_HIGH_VOLTAGE);
	}
}

void TrackerLightsController::turnOffLeftLight()
{
	if (m_LeftCamera)
	{
		m_LeftCamera->WriteRegister(GPIO_2_CTL_ADDRESS, GPIO_LOW_VOLTAGE);
	}
}

void TrackerLightsController::turnOffRightLight()
{
	if (m_RightCamera)
	{
		m_RightCamera->WriteRegister(GPIO_2_CTL_ADDRESS, GPIO_LOW_VOLTAGE);
	}
}