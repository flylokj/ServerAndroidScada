#include "Protocol.h"

int bAutoAck = 0;
int bAutoRead = 0;

typedef struct tagProtocolData{
	ProtocolConf conf;
	int nTimeout;
    quint8 revBuf[ MAX_PROTOCL_BUF_SZ ];
	uint16 revIndex;
    quint8 senBuf[ MAX_PROTOCL_BUF_SZ * 2 ];
	uint16 cmdLen;
	mbyte bESCGet;
}ProtocolData,*LPProtocolData;

static ProtocolData g_protocol = {0};
 
int Asiic2Hex( mbyte* pstr,uint16 sz,uint32* phex )
{
	mbyte* pc;
	*phex = 0;
	pc = pstr + sz - 1;
	while( pc >= pstr )
	{
		*phex <<= 4;
		if( *pc >= 'A' && *pc <= 'F' )
			*phex |= *pc - 55;
		else if( *pc >= '0' && *pc <= '9' )
			*phex |= *pc - 48;
		else
			return 0;
		pc--;
	}
	return 1;
}

__inline void SendNAK()
{
	uint16 nak = NAK;
	if( g_protocol.conf.write )
		g_protocol.conf.write( &nak, 1 );
}

__inline void SendACK()
{
	uint16 enq = ACK;
    if(bAutoAck==0)
        return;
	if( g_protocol.conf.write )
		g_protocol.conf.write( &enq, 1 );
}

//int API_ProtocolSend( mbyte* pData, uint16 sz )
//{
//	mbyte* p;
//	mbyte* pEnd;
//	mbyte sendBuf[21];
//	mbyte* ps = sendBuf;
//	mbyte check;
	
//	check = 0;
//	*ps++ = STX;
//	pEnd = pData + sz;
//	p = pData;
//	while( p < pEnd )
//	{
//		/**/switch( *p )
//		{
//		case STX:
//		case ENQ:
//		case ETX:
//		case ESC:
//			*ps++ = ESC;
//		break;
//		}
//		/*if( *p == STX
//			||  *p == ENQ
//			|| 	*p == ETX
//			|| 	*p == ESC )
//		{
//			*ps++ = ESC;
//		}*/
//		check += *p;
//		*ps++ = *p++;
//	}
//	*ps++ = check;
//	*ps++ = ETX;
//	if( g_protocol.conf.write )
//		g_protocol.conf.write( sendBuf, ps -  sendBuf );
//	return 1;
//}


int API_ProtocolSend( mbyte* pData, uint16 sz )
{
    mbyte* p;
    mbyte* pEnd;
    mbyte sendBuf[MAX_PROTOCL_BUF_SZ];
    mbyte* ps = sendBuf;
    mbyte check;

    check = 0;
    *ps++ = STX;
    pEnd = pData + sz;
    p = pData;
    while( p < pEnd )
    {
        /**/switch( *p )
        {
        case STX:
        case ENQ:
        case ETX:
        case ESC:
            *ps++ = ESC;
        break;
        }
        /*if( *p == STX
            ||  *p == ENQ
            || 	*p == ETX
            || 	*p == ESC )
        {
            *ps++ = ESC;
        }*/
        check += *p;
        *ps++ = *p++;
    }
    *ps= check;
    if(check == STX || check == ENQ || check == ETX || check == ESC)
        *ps++ = ESC;
    *ps++ = check;
    *ps++ = ETX;
    if( g_protocol.conf.write )
        g_protocol.conf.write( sendBuf, ps -  sendBuf );

    return 1;
}

extern int diaoni;
extern int diaoni2;
extern int diaoni3;
extern int diaoni4;
void ProcessData()
{
	//���У���
    quint8 senBuf[ MAX_PROTOCL_BUF_SZ ];
	mbyte cmd;
	mbyte coil;
	uint16 addr;
	mbyte sz;
    uint16 dataGet;
    quint8* pData;
    quint8* pc;
	int i;
	mbyte checksum = 0;

	for ( i =0; i < g_protocol.revIndex - 1; i++ )
		checksum += g_protocol.revBuf[i];
	if( ( checksum & 0x00ff ) != g_protocol.revBuf[ g_protocol.revIndex - 1 ] )
    {
        SendNAK();
        diaoni = 1;
        diaoni2 = checksum;
        diaoni3 = g_protocol.revBuf[ g_protocol.revIndex - 1 ];
        diaoni4 = g_protocol.revIndex;
    }
	else
	{
		pc = g_protocol.revBuf;
		cmd = *pc++;
		if( cmd != CMD_PRO_READ && cmd != CMD_PRO_WRITE )
			coil = (*pc++) & 0x00ff;
		addr = ( *pc++ & 0x00ff ) << 8;
		addr |= *pc++ & 0x00ff;
		sz = (*pc++) & 0x00ff;
		pData = pc;
		
		switch( cmd )
		{
		case CMD_READ:
			pData = senBuf;
            if(sz!=1)
                break;
            if(coil == dcoilD)
            {
                *pData++ = CMD_WRITE;
                *pData++ = dcoilD;
                *pData++ = (addr>>8)&0xff;
                *pData++ = addr&0xff;
                *pData++ = 1;
                while( sz-- )
                {
                    if( !API_ReadData( coil, addr++, &dataGet ) )
                        break;
                    else
                    {
                        static uint16 temp = 0;
                        dataGet = temp++;//debug!!!!!!!!!!!!!!!
                        *pData++ = (dataGet & 0xff00 ) >> 8;
                        *pData++ = dataGet & 0x00ff;
                    }
                }
            }
            else if(coil == dcoilX)
            {

            }
            else if(coil == dcoilY)
            {

            }

            if( ( ( sz + 1 )&0xff ) == 0 )
			{
                if(bAutoRead == 0)
                    break;
				if( !API_ProtocolSend( senBuf, pData - senBuf ) )
					SendNAK();
			}else
				SendNAK();
			break;
		case CMD_WRITE:
			while( sz-- )
			{
				dataGet = ( *pData++ & 0x00ff ) << 8;
				dataGet |= ( *pData++ & 0x00ff );
				if( !API_WriteData( coil, addr++, dataGet ) )
					break;
			}
            if( ( ( sz + 1 )&0xff ) == 0 )
            {
                SendACK();
                diaoni = 2;
            }
			else
            {
                SendNAK();
                diaoni = 3;
            }
			break;
		case CMD_FORCEON:
			if( API_WriteBit( coil, addr, 1 ) )
				SendACK();
			else
				SendNAK();
			break;
		case CMD_FORCEOFF:
			if( API_WriteBit( coil, addr, 0 ) )
				SendACK();
			else
				SendNAK();
			break;
		case CMD_PRO_READ:
			if( API_IsPLCRuning() )
			{
				SendNAK();
				break;
			}
			pData = senBuf;
			while( sz-- )
			{
				if( !API_ProGramRead( addr++, &dataGet ) )
					break;
				else
				{
					*pData++ = (dataGet & 0xff00 ) >> 8;
					*pData++ = dataGet & 0x00ff;
				}
			}
            if( ( ( sz + 1 )&0xff ) == 0 )
			{
				if( !API_ProtocolSend( senBuf, pData - senBuf ) )
					SendNAK();
			}else
				SendNAK();
			break;
		case CMD_PRO_WRITE:
			if( API_IsPLCRuning() )
			{
				SendNAK();
				break;
			}
			while( sz-- )
			{
				dataGet = ( *pData++ & 0x00ff ) << 8;
				dataGet |= ( *pData++ & 0x00ff );
				if( !API_ProGramWrite( addr++, dataGet ) )
					break;
			}
            if( ( ( sz + 1 )&0xff ) == 0 )
				SendACK();
			else
				SendNAK();
			break;
		default:
			SendNAK();
			break;
		}
	}
}


int API_Protocol( mbyte* pData, uint16 sz )
{
    //static bool stxFlag = false;
	while( sz-- )
	{
		if( g_protocol.revIndex >= MAX_PROTOCL_BUF_SZ )
		{
			SendNAK();
			g_protocol.revIndex = 0;
		}else
		{
			switch( *pData )
			{
			case ENQ:
				if( g_protocol.bESCGet )
				{
					g_protocol.revBuf[ g_protocol.revIndex++] = *pData;
					g_protocol.bESCGet = 0;
				}else
				{
					g_protocol.revIndex = 0;
					SendACK();
				}
				break;
			case STX:
				if( g_protocol.bESCGet )
				{
					g_protocol.revBuf[ g_protocol.revIndex++] = *pData;
					g_protocol.bESCGet = 0;
				}else
				{
                    //stxFlag = true;
					g_protocol.revIndex = 0;
				}
					
				break;
			case ESC:
				if( g_protocol.bESCGet )
				{
					g_protocol.revBuf[ g_protocol.revIndex++] = *pData;
					g_protocol.bESCGet = 0;
				}else
					g_protocol.bESCGet = 1;
				break;
			case ETX:
				if( g_protocol.bESCGet )
				{
					g_protocol.revBuf[ g_protocol.revIndex++] = *pData;
					g_protocol.bESCGet = 0;
				}else
				{
                    //if(stxFlag)
					{
                        //stxFlag = false;
						ProcessData();
					}
                    //else
                        //g_protocol.revIndex = 0;
					
				}
				break;
			default:
				g_protocol.revBuf[ g_protocol.revIndex++] = *pData;
				break;
			}
		}
		++pData;
	}
	return 1;//����1��ʾ�Ѿ��ɸú���ʶ�𲢴���
}

void API_InitProtocol( void )
{
	memset( &g_protocol, 0, sizeof( g_protocol ) );
}

void SetProtocolConf( LPProtocolConf conf, unsigned int uFlag )
{
	if( PROTOCOL_CONF_WRITEFUN & uFlag )
		g_protocol.conf.write = conf->write;
	if( PROTOCOL_CONF_SETTIMERFUN & uFlag )
		g_protocol.conf.setTimer = conf->setTimer;
	if( PROTOCOL_CONF_TIMEOUTCOUNT & uFlag )
		g_protocol.conf.nTimeOutCount = conf->nTimeOutCount;
	if( PROTOCOL_CONF_TIMEOUT & uFlag )
		g_protocol.conf.lTimerout = conf->lTimerout;
}




