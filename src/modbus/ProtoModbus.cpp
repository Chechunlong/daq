#include <string>
#include <string.h>
#include <stdio.h> 
#include <iostream>
#include <stdlib.h>
#include "common.h"
#include <stdlib.h>

#ifdef WIN32
	#include <windows.h>
	#include <WinSock2.h>
#else
	#include <arpa/inet.h>
#endif

#include "ProtoModbus.h"


/*
CRC-16��ļ��㲽��
1��	��16λ�Ĵ���Ϊʮ������FFFF(��ȫΪ1)���ƴ˼Ĵ���ΪCRC�Ĵ�����
2��	��һ��8λ������16λCRC�Ĵ����ĵ�λ����򣬰ѽ������CRC�Ĵ�����
3��	�ѼĴ�������������һλ(����λ)����0����λ��������λ(�Ƴ�λ)��
4��	������λΪ0������3��(�ٴ���λ)��
������λΪ1��CRC�Ĵ��������ʽA001(1010 0000 0000 0001)�������
5��	�ظ�����3��4��ֱ������8�Σ���������8λ����ȫ�������˴���
6��	�ظ�����2������5��������һ��8λ�Ĵ���
7��	���õ���CRC�Ĵ�����ΪCRC�룬���ֽ���ǰ�����ֽ��ں�
*/

//CRC16��modbus RTUר��
uint16 calccrc(uint8 crcbuf, uint16 crc)
{
	int i; 
	crc=crc ^ crcbuf;
	for(i=0;i<8;i++)
	{
		uint8 chk;
		chk=crc&1;
		crc=crc>>1;
		crc=crc&0x7fff;
		if (chk==1)
			crc=crc^0xa001;
		crc=crc&0xffff;
	}
	return crc; 
}

uint16 CRC_16(const uint8 *buf, int len)//�ṹΪINTEL˳�򣬵��ֽ���ǰ�����ֽ��ں�
{
	int  i;
	unsigned short crc;
	crc=0xFFFF;
	for (i=0;i<len;i++)
	{	
		crc=calccrc(*buf,crc);
		buf++;
	}
	return crc;
}



uint16 CProtoModbusCom::GetReqBufSize(uint8 funcode)
{
	if(funcode < 5)
	{
		return 8;//1+5+2
	}
	else
	{
		return 0;//������������ʱ������
	}
}

uint16 CProtoModbusTcp::GetReqBufSize(uint8 funcode)
{
	if(funcode < 5)
	{
		return 12;//7+5
	}
	else
	{
		return 0;//������������ʱ������
	}
}

uint16 CProtoModbusTcp::GetRspBufSize(uint8 funcode, uint16 cnt)
{	
	uint16 wLen=0;
	switch(funcode)//���ݳ���
	{	
	case 0x01:
	case 0x02:
		wLen = (cnt < 1) ? 9 : 9 + (cnt-1)/8 + 1;//7+2=9,���������ݳ���			
		break;
	case 0x03:
	case 0x04:
		wLen = 9 + cnt*2;
		break;
	default :
		wLen = 0;
	}
	return wLen;
}

uint16 CProtoModbusCom::GetRspBufSize(uint8 funcode, uint16 cnt)
{	
	uint16 wLen=0;
	switch(funcode)//���ݳ���
	{	
	case 0x01:
	case 0x02:
		wLen = (cnt < 1) ? 5 : 5 + (cnt-1)/8 + 1;			
		break;
	case 0x03:
	case 0x04:
		wLen = 5 + cnt*2;
		break;
	default :
		wLen = 0;
	}
	return wLen;
}

void CProtoModbusTcp::PackPollingReq(uint8 devaddr
										, uint8 funcode
										, uint16 startaddr
										, uint16 cnt
										, uint8* &out
										, uint16 &outlen)
{
	static uint16 wNo = 1;
	*(uint16*)(out) = htons(wNo++);//�൱�ڰ���ţ���ʱ����
	out += sizeof(uint16);
	*(uint16*)(out) = htons(0);//0 -> modbus
	out += sizeof(uint16);
	*(uint16*)(out) = htons(GetReqBufSize(funcode)-6);//���ֶκ�������ݳ���
	out += sizeof(uint16);
	*(uint8*)out = devaddr; //�豸��ַ
	out += sizeof(uint8);

	uint16 pdulen = 0;
	PackPdu(funcode, startaddr, cnt, out, pdulen);//PDU

	outlen = GetReqBufSize(funcode);
}

void CProtoModbusCom::PackPollingReq(uint8 devaddr
										, uint8 funcode
										, uint16 startaddr
										, uint16 cnt
										, uint8* &out
										, uint16 &outlen)
{
	uint8 *pTemp = out;

	*(uint8*)pTemp = devaddr; //�豸��ַ
	pTemp += sizeof(uint8);

	uint16 wLen = 0;
	PackPdu(funcode, startaddr, cnt, pTemp, wLen);//PDU
	pTemp += wLen;
	
	*(uint16*)pTemp = CRC_16(out, GetReqBufSize(funcode)-2);//crcУ��

	outlen = GetReqBufSize(funcode);

}

bool CProtoModbusTcp::ParsePollingRsp(uint8 *in
									 , uint16 inlen
									 , uint8 &devaddr
									 , uint8 &funcode
									 , uint8* &out
									 , uint16 &outlen)
{
	uint8 *pdu = NULL;
	uint16 pdulen = 0;
	if(!GetPdu(in, inlen, pdu, pdulen))
	{
		return false;
	}

	devaddr = *(uint8*)(in+6);

	ParsePdu(pdu, pdulen, funcode, out, outlen);

	return true;
}

bool CProtoModbusCom::ParsePollingRsp(uint8 *in
									  , uint16 inlen
									  , uint8 &devaddr
									  , uint8 &funcode
									  , uint8* &out
									  , uint16 &outlen)
{
	uint8 *pdu = NULL;
	uint16 pdulen = 0;

	if(!GetPdu(in, inlen, pdu, pdulen))
	{
		return false;
	}

	devaddr = *(uint8*)in;

	ParsePdu(pdu, pdulen, funcode, out, outlen);

	return true;
}

void CProtoModbus::PackPdu(uint8 funcode
			 , uint16 startaddr
			 , uint16 cnt
			 , uint8 *pdu
			 , uint16 &pdulen)
{
	pdulen = 0;
	*(uint8*)pdu = funcode;
	pdulen += sizeof(uint8);
	*(uint16*)(pdu+pdulen) = htons(startaddr);
	pdulen += sizeof(uint16);
	*(uint16*)(pdu+pdulen) = htons(cnt);
	pdulen += sizeof(uint16);
}

void CProtoModbus::ParsePdu(uint8 *pdu
			  , uint16 pdulen
			  , uint8 &funcode
			  , uint8* &out
		      , uint16 &outlen)
{
	uint8 *pTemp = pdu;
	funcode = *(uint8*)pTemp;
	pTemp += sizeof(uint8);
	outlen  = *(uint8*)pTemp;
	pTemp += sizeof(uint8);
	out = pTemp;
}

/*
tcp ADU��ʽ��7(MBAPͷ)+pdu
*/
bool CProtoModbusTcp::GetPdu( uint8 *adu
								, uint16 adulen
								, uint8* &pdu
								, uint16 &pdulen)
{
	if(adu)
	{
		pdu = adu + 7;//ȡ��5��MBAPͷ
		pdulen = adulen - 7;
		return true;
	}

	return false;
}

/*
����ADU��ʽ��1(�豸��ַ)+PDU+2(У��)
*/

bool CProtoModbusCom::GetPdu( uint8 *adu
								, uint16 adulen
								, uint8* &pdu
								, uint16 &pdulen)
{
	if(adu)
	{
		uint16 wCrc = *(uint16*)(adu+adulen-2);
		if(CRC_16(adu, adulen-2) != wCrc)
		{
			return false;
		}
		pdu = adu + 1;//����1��ͷ
		pdulen = adulen - 3;
		return true;
	}

	return false;
}
