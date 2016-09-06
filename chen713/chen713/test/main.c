#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdl\sdl.h"
#include "mib\mib.h"
#include "common.h"
#include "entity.h"

Entity ent[4];
int main()
{
	int i=0,j=0;
	unsigned short test_send_id;
	unsigned short test_receive_id;
	unsigned short tx_data_0[(750-2)/2];  //len<=NET_PAYLOAD_SIZE(748)��
	unsigned short tx_data_1[(750-2)/2];
	unsigned short tx_data_2[(750-2)/2];
	unsigned short tx_data_3[(750-2)/2];

	Signal* sig;

	for(i = 0; i < 4; i++)
	{		
		ent[i].mib.local_id = i;//yun�����Ҫ����ǰ�棬������ܳ�ʼ����Ӧ��ģ��
		EntitySetup(&ent[i]);
		EntityInit(&ent[i]);
		
	}
	//yun�����Բ�ͬ�ڵ�֮�� 
	test_send_id = 0;  //���ͽڵ�
	test_receive_id = 1;  //���սڵ�
	//yun:��-��
	//ent[test_send_id].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[test_receive_id].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //��-��
	
	//ent[0].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[1].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //0��-1��
	//ent[0].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[2].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //0��-2��

	//ent[1].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[0].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //1��-0��
	//ent[1].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[2].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //1��-2��

	//ent[2].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[0].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //2��-0��
	//ent[2].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[1].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //2��-1��

	//���� �ظ�ack
	//ent[test_receive_id].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[test_send_id].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //��-��
	//ent[0].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[1].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //0-1
	//ent[0].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[2].dlc_tx.dlc_tx_ctrl.ack_rx_ind;	 //0-2

	//ent[1].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[0].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //1-0
	//ent[1].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[2].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //1-2

	//ent[2].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[0].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //2-0
	//ent[2].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[1].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //2-1

	//�ڴ��ʼ��
	MemInitNet();
	MemInitMac();
	//��������
	for (i = 0; i < (750-2)/2; i++)
	{
		tx_data_0[i]=i;
		tx_data_1[i]=(i+1000);
		tx_data_2[i]=(i+2000);
		tx_data_3[i]=(i+2000);
	}
	
	while(1)
	{
		if(0 == j){
			
			//�ȷ�����ntx
			sig = &ent[0].ntx.data_tx_req;
			((Data_Tx_Req_Param*)sig->param)->xdat=tx_data_0;//������
			((Data_Tx_Req_Param*)sig->param)->xlen= sizeof(tx_data_0);//����
			//((Data_Tx_Req_Param*)sig->param)->xda=test_id;//Ŀ�ĵ�ַ   �㲥
			((Data_Tx_Req_Param*)sig->param)->xda=1;//Ŀ�ĵ�ַ ����
			((Data_Tx_Req_Param*)sig->param)->xpri=1;//���ȼ�
			((Data_Tx_Req_Param*)sig->param)->hello_flag=0;//hello����־
			printf("\n----main(j=0)----main.c ��ʼ���͵�1��ҵ�������\n");
			sig->func(sig);
		}
		if(8 == j){
			
			//�ȷ�����ntx
			sig = &ent[2].ntx.data_tx_req;
			((Data_Tx_Req_Param*)sig->param)->xdat=tx_data_1;//������
			((Data_Tx_Req_Param*)sig->param)->xlen= sizeof(tx_data_1);//����
			((Data_Tx_Req_Param*)sig->param)->xda=1;//Ŀ�ĵ�ַ  
			((Data_Tx_Req_Param*)sig->param)->xpri=1;//���ȼ�
			((Data_Tx_Req_Param*)sig->param)->hello_flag=0;//hello����־
			printf("\n----main(j=8)-----main.c ��ʼ���͵�2��ҵ�������\n");
			sig->func(sig);
		}
		if(20 == j){
			//�ȷ�����ntx
			sig = &ent[3].ntx.data_tx_req;
			((Data_Tx_Req_Param*)sig->param)->xdat=tx_data_2;//������
			((Data_Tx_Req_Param*)sig->param)->xlen= sizeof(tx_data_2);//����
			((Data_Tx_Req_Param*)sig->param)->xda=0;//Ŀ�ĵ�ַ   
			((Data_Tx_Req_Param*)sig->param)->xpri=1;//���ȼ�
			((Data_Tx_Req_Param*)sig->param)->hello_flag=0;//hello����־
			printf("\n----main(j=20)-----main.c ��ʼ���͵�3��ҵ�������\n");
			sig->func(sig);
		}
		// sdl execsignal
		for(i=0 ; i<50; i++)
		{
			CheckSignal(&ent[0].sdlc);
			CheckSignal(&ent[1].sdlc);
			CheckSignal(&ent[2].sdlc);
			CheckSignal(&ent[3].sdlc);
		}
		//yun:mac����dlc_tx_ctrlҪ����  
		if(0<=j && 13>j){
			printf("---mian.c----(0<=j && 13>j):0->1Ҫ����\n");
			sig = &ent[0].dlc_tx.dlc_tx_ctrl.dlc_tx_ind;
			sig->func(sig);
			ent[0].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[1].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //0��-1��
			ent[1].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[0].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //1-0
		}else if(8<=j && 26>j){
			printf("---mian.c---(8<=j && 26>j):2->1Ҫ����\n");
			sig = &ent[2].dlc_tx.dlc_tx_ctrl.dlc_tx_ind;
			sig->func(sig);
			ent[2].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[1].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //2��-1��
			ent[1].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[2].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //1-2
			
		}else if(20<=j && 40>j){
			printf("---mian.c---(20<=j && 40>j):3->0Ҫ����\n");
			sig = &ent[3].dlc_tx.dlc_tx_ctrl.dlc_tx_ind;
			sig->func(sig);
			ent[3].dlc_tx.dlc_tx_ctrl.dlc_rx_ind = &ent[0].dlc_rx.dlc_rx_ctrl.dlc_rx_ind;  //2��-1��
			ent[0].dlc_rx.dlc_rx_ctrl.ack_tx_req = &ent[3].dlc_tx.dlc_tx_ctrl.ack_rx_ind;  //1-2
			
		}
		j++; 
		if (40==j)
		{
			break;
		}
		
	}

	return 0;	
}