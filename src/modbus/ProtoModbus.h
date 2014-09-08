#ifndef _PROTOCOLMODBUS_H_
#define _PROTOCOLMODBUS_H_

#include "daqtypes.h"
//base class
//PDU(Э�����ݵ�Ԫ)���ֽ������ദ��ADU(Ӧ�����ݵ�Ԫ)������������������ദ��
class CProtoModbus
{
public:

	virtual void PackPollingReq(uint8 devaddr
							, uint8 funcode
							, uint16 startaddr
							, uint16 cnt
							, uint8* out
							, uint16 &outlen) = 0;

	virtual bool ParsePollingRsp(uint8 *in
						, uint16 inlen
						, uint8 &devaddr
						, uint8 &funcode
						, uint8* &out
						, uint16 &outlen) = 0;

	virtual uint16 GetReqBufSize(uint8 funcode) = 0;
	virtual uint16 GetRspBufSize(uint8 funcode, uint16 cnt) = 0;

protected:

	void PackPdu(uint8 funcode
				, uint16 startaddr
				, uint16 cnt
				, uint8 *pdu
				, uint16 &pdulen);

	void ParsePdu(uint8 *pdu
				, uint16 pdulen
				, uint8 &funcode
				, uint8* &out
				, uint16 &outlen);

	virtual bool GetPdu(uint8 *adu
					, uint16 adulen
					, uint8* &pdu
					, uint16 &pdulen) = 0;

};

class CProtoModbusTcp : public CProtoModbus
{
public:
	virtual void PackPollingReq(uint8 devaddr
							, uint8 funcode
							, uint16 startaddr
							, uint16 cnt
							, uint8* out
							, uint16 &outlen);
	virtual bool ParsePollingRsp(uint8 *in
							, uint16 inlen
							, uint8 &devaddr
							, uint8 &funcode
							, uint8* &out
							, uint16 &outlen);

	virtual uint16 GetReqBufSize(uint8 funcode);
	virtual uint16 GetRspBufSize(uint8 funcode,uint16 lenaddr);	

private:

	//��ȡЭ�����ݵ�Ԫ(PDU)
	virtual bool GetPdu( uint8 *adu
					, uint16 adulen
					, uint8* &pdu
					, uint16 &pdulen);
};

class CProtoModbusCom : public CProtoModbus
{
public:
	virtual void PackPollingReq(uint8 devaddr
							, uint8 funcode
							, uint16 startaddr
							, uint16 cnt
							, uint8* out
							, uint16 &outlen);

	virtual bool ParsePollingRsp(uint8 *in
							, uint16 inlen
							, uint8 &devaddr
							, uint8 &funcode
							, uint8* &out
							, uint16 &outlen);

	virtual uint16 GetReqBufSize(uint8 funcode);
	virtual uint16 GetRspBufSize(uint8 funcode,uint16 lenaddr);

private:
	
	//��ȡЭ�����ݵ�Ԫ(PDU)
	virtual bool GetPdu( uint8 *adu
					, uint16 adulen
					, uint8* &pdu
					, uint16 &pdulen);
};


extern uint16 CRC_16(const uint8 *buf, int len);

#endif
