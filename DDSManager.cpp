#include <stdio.h>
#include <stdlib.h>
#include "DDSManager.h"
#include "Simulation.h"
//#include "ndds/ndds_cpp.h"

#pragma execution_character_set("utf-8")

#define RECV_MSG_FIXED(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ##__VA_ARGS__)

DDSManager * DDSManager::sInstance = nullptr;

DDSManager::DDSManager()
{
	pDDSCore = DDSCore::GetInstance();
}

DDSManager * DDSManager::GetInstance()
{
	if (sInstance == nullptr)
		sInstance = new DDSManager();
	return sInstance;
}

void DDSManager::close()
{
	pDDSCore->close();
	//delete sInstance;
	sInstance = nullptr;
}
DDSManager::~DDSManager()
{
	//pDDSCore->close();
}

bool DDSManager::OnRecvUavSwitchCmd(std::string topicName, unsigned char * pA, int nSizeA)
{
	//printf("topicName:%s", topicName);
	std::cout << "topicName:" << topicName << endl;
	printf("recvLen:%d\n", nSizeA);
	int index = 0;
	for (index = 0; index < nSizeA; index++)
	{
		printf("%d", *(pA + index));
	}
	printf("\n");
	return true;
}

bool DDSManager::RTIDDSRecv(std::string topicName, unsigned char* pA, int nSizeA)
{
    p_Simulation->RecvExternalDDSCmd(topicName.data(), pA, nSizeA);

    return true;
}

//----6
void DDSManager::InitRecv()
{
	pDDSCore->RegisterTopicFixedListener(Topic_RadiationNoise, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_TargetMotion, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_LineSpectrum, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	// ·ÂÕæ²½½ø
	pDDSCore->RegisterTopicFixedListener(Topic_SimulationStep, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_OceanNoise, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_Environment, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_SonarArray, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	// ÉùÄÉ×´Ì¬
	pDDSCore->RegisterTopicFixedListener(Topic_SonarState, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_SonarConfig, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_PropagationLoss, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	// ÉùÄÉ¼ì²â½á¹û
	pDDSCore->RegisterTopicFixedListener(Topic_DetectionResult, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_SignalPacket, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_SpectrumData, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
	pDDSCore->RegisterTopicFixedListener(Topic_MotionState, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
    // ·É»ú×´Ì¬
    pDDSCore->RegisterTopicFixedListener(Topic_AircraftState , RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
    // ÓãÀ××´Ì¬
    pDDSCore->RegisterTopicFixedListener(Topic_TorpedoStepOutput, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
    // Ô¤²âÎ»ÖÃ
    pDDSCore->RegisterTopicFixedListener(Topic_TrackingOutput, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));
    // Ö÷¶¯ÉùÄÉ½Ó´¥/»Ø²¨
    pDDSCore->RegisterTopicFixedListener(Topic_ActiveSonar, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));

    //--------------------------
    pDDSCore->RegisterTopicFixedListener(Topic_AMTIMode, RECV_MSG_FIXED(DDSManager::RTIDDSRecv, this));

}

void DDSManager::InitSend()
{
	pDDSCore->RegisterWriter("Example DDS_Octets");

//    pDDSCore->RegisterWriter(Topic_ActiveSonarConfig);
//    pDDSCore->RegisterWriter(Topic_TorpedoLaunch);
}

bool DDSManager::sendData(const std::string & topicName, void * buf, int len)
{
	pDDSCore->SendTopicRawMsg(topicName, buf, len);
	return true;
}

bool DDSManager::sendDataNoLen(const std::string & topicName, void * buf)
{
	pDDSCore->SendTopicRawMsgLenbyConfig(topicName, buf);
	return true;
}




