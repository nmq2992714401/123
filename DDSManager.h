#pragma once
#include "DDSCore.h"
#include <QTimer>

#pragma execution_character_set("utf-8")

#define MSG_FIXED(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, ##__VA_ARGS__)
class DDSManager
{
public:
	DDSManager();
	~DDSManager();
	static DDSManager *GetInstance(); //ЕЅСа
	void InitRecv();
	void InitSend();
    void close();
	bool sendData(const std::string & topicName, void * buf, int len);
	bool sendDataNoLen(const std::string & topicName, void * buf);

private:
	static DDSManager *sInstance;
	DDSCore* pDDSCore;
	bool OnRecvUavSwitchCmd(std::string topicName, unsigned char * pA, int nSizeA);
	bool RTIDDSRecv(std::string topicName, unsigned char* pA, int nSizeA);

};
