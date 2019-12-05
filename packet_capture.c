#define WIN32
#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>
#include <pcap\pcap.h>
#include <pcap.h>
#include <string.h>
#include <WinSock2.h>

void packet_handler(u_char* param, const struct pcap_pkthdr* h, const u_char* data);
//��Ŷ�� ���� ���� ���¿��� �а� ó���ϴ� �Լ�

typedef struct Ethernet_Header//�̴��� ��� ����ü
{
	u_char des[6];//������ MAC �ּ�
	u_char src[6];//�۽��� MAC �ּ�
	short int ptype;//�ڿ� ���� ��Ŷ�� �������� ����(��:ARP/IP/RARP)
			//IP ����� ���� ��� : 0x0800
			//ARP ����� ���� ��� : 0x0806
			//RARP ����� ���� ��� : 0x0835
}Ethernet_Header;//�θ� �̸� ����(����)

typedef struct ipaddress
{
	u_char ip1;
	u_char ip2;
	u_char ip3;
	u_char ip4;
}ip;

//IP ���������� ����� ������ ����ü ����
typedef struct IPHeader
{
	u_char HeaderLength : 4;//��� ���� *4
	u_char Version : 4;//IP v4 or IPv6
	u_char TypeOfService;//���� ����
	u_short TotalLength;//��� ���� + ������ ����/
	u_short ID;//������ ��Ʈ�� Identification
	u_short FlagOffset;//�÷��� + �����׸�Ʈ ������

	u_char TimeToLive;//TimeToL
	u_char Protocol;//�������� ����(1. ICMP 2. IGMP 6. TCP 17:UDP;
	u_short checksum;
	ip SenderAddress;
	ip DestinationAddress;
	u_int Option_Padding;

	unsigned short source_port;
	unsigned short dest_port;
}IPHeader;


typedef struct CheckSummer
{
	//0    2byte       2byte   32
	// [   4500   ][   003c   ] Version, HeaderLength, TypeOfService / TotalLength
	// [   11e5   ][   0000   ] Identification / Flag, FragmentOffset
	// [   8001   ][          ] TimeToLive, Protocol / HeaderChecksum
	// [   7c89   ][   19a4   ] Source Address
	// [   7c89   ][   19a3   ] Destination Address
	// �� ��� ������ ���� HeaderChecksum ���� ���� ���, ��Ŷ�� �����̴�.
	// �׷��� �� ���ϸ� 2037b �� 2����Ʈ ũ�⸦ �Ѱ� �ȴ�.
	// �׷��� �� 037b�� ������ ���� �÷ο� �� 2�� �ڿ� ���Ѵ�.
	//     037b
	//  +     2
	//  ��������
	//     037d
	// �׸��� ���� ��� ��� ��(037d)�� ���� ���·� ���Ѵ�.
	// (1�� ���� = 0�� 1��, 1�� 0����)
	// 037d = 0000 0011 0111 1101
	// ���� = 1111 1100 1000 0010
	// 16�� = fc82
	// �׷��Ƿ� ����� �κп��� fc82�� ���� �ȴ�.

	/*
	u_char = 1 byte
	u_short = 2 byte
	int = 4 byte
	*/
	//������ ���� ������ Version�� ��� ���� �������� �޾ƾ� ���������� ��� ���̿� ������ ���´�.
	/*
	u_char = 1 byte
	u_short = 2 byte
	int = 4 byte
	*/
	u_short part1;
	u_short part2;
	u_short part3;
	u_short part4;
	u_short part5;
	u_short checksum;
	u_short part6;
	u_short part7;
	u_short part8;
	u_short part9;

}CheckSummer;

void main()
{
	pcap_if_t* allDevice; //ã�Ƴ� ����̽��� LinkedList�� ����, �� �� ù ��° ������Ʈ�� ���� ���� ����
	pcap_if_t* device; //Linked List�� ���� ������Ʈ�� ���� ����
	char errorMSG[256]; //���� �޽����� ���� ���� ����
	char counter = 0;

	pcap_t* pickedDev; //����� ����̽��� �����ϴ� ����

					   //1. ��ġ �˻� (ã�Ƴ� ����̽��� LinkedList�� ����)
	if ((pcap_findalldevs(&allDevice, errorMSG)) == -1)//���� �����ÿ��� 1 ����������, pcap_findallDevice�� ���°� ���� ����Ʈ�̹Ƿ� �ּҷ� �־�� ��.
													   //pcap_if_t�� int���¸� ��ȯ�ϸ�, -1�� ���� ���, ����̽��� ã�� ������ ����̴�.
		printf("��ġ �˻� ����");

	//2. ��ġ ���
	int count = 0;
	for (device = allDevice; device != NULL; device = device->next)
		//dev�� allDevice�� ù ���� �ּҸ� ������, dev�� ���� NULL(��)�� ��� ����, dev�� �� for���� ���� �ּҰ����� ��ȯ
	{
		printf("��%d �� ��Ʈ��ũ ī�妡����������������������������������������������������\n", count);
		printf("������� ���� : %s\n", device->name);
		printf("������� ���� : %s\n", device->description);
		printf("��������������������������������������������������������������������������\n");
		count = count + 1;
	}

	//3. ��Ʈ��ũ ī�带 �����ϰ� ���õ� ����̽��� ������ ��Ŷ �����ϱ�
	printf("��Ŷ�� ������ ��Ʈ��ũ ī�带 ���� �ϼ��� : ");
	device = allDevice;//ī�带 �������� �ʰ� �׳� ù ��° ī��� ��������.

	int choice;
	scanf_s("%d", &choice);
	for (count = 0; count < choice; count++)
	{
		device = device->next;
	}

	//��Ʈ��ũ ��ġ�� ����, ������ ��Ŷ ���� �����Ѵ�.
	pickedDev = pcap_open_live(device->name, 65536, 0, 1000, errorMSG);
	//��ī���� �̸�, ������ ��Ŷ ũ��(�ִ� 65536), ���ι̽�ť����(��Ŷ ���� ���) ����, ��Ŷ ��� �ð�, ���� ������ ������ ����)

	//4. ��ī�� ����Ʈ ������ ������ �޸𸮸� ����ش�.
	pcap_freealldevs(allDevice);

	//5. ������ ��Ʈ��ũ ī�忡�� ��Ŷ�� ���� ĸ�� �� �Լ��� ����� ĸ�ĸ� �����Ѵ�.
	pcap_loop(pickedDev, 0, packet_handler, NULL);
}

//�Ʒ����� ����� �� �ֵ�����Ŷ �ڵ鷯�� �����.
void packet_handler(u_char* param, const struct pcap_pkthdr* h, const u_char* data)
//���� = �Ķ����, ��Ŷ ���, ��Ŷ ������(������ MAC �ּ� �κ� ����)
{
#define IPHEADER 0x0800
#define ARPHEADER 0x0806
#define RARPHEADER 0x0835
	struct tm ltime;
	char timestr[16];
	time_t local_tv_sec;



	/*
	 * unused variables
	 */
	(VOID)(param);
	(VOID)(data);

	/* convert the timestamp to readable format */
	local_tv_sec = h->ts.tv_sec;
	localtime_s(&ltime, &local_tv_sec);
	strftime(timestr, sizeof timestr, "%H:%M:%S", &ltime);

	printf("�ð�: %s,%.6d len:%d\n", timestr, h->ts.tv_usec, h->len);


	//�ҽ� ���� �� �������� ���� ����� ���ڷ� �ٲ۴�.
	Ethernet_Header* EH = (Ethernet_Header*)data;//data �ּҿ� ����� 14byte �����Ͱ� ����ü Ethernet_Header ���·� EH�� ����ȴ�.
	short int type = ntohs(EH->ptype);
	//EH->ptype�� �� ����� ������ ���ϹǷ�,
	//�̸� ��Ʋ ����� �������� ��ȯ(ntohs �Լ�)�Ͽ� type�� �����Ѵ�.
	printf("���� ��Ŷ : %04x\n", EH->ptype);
	//�������� �׳� ��Ʈ��ũ�� ���� 1����Ʈ�� �ʰ��ϴ� �����͸�
	//���� ���� ����  �׻� ��Ʋ ����� �������� ��ȯ �� �־�� �Ѵٰ� �ܿ���.

	printf("����������������������������������������������������\n");
	printf("��  Src MAC : %02x-%02x-%02x-%02x-%02x-%02x\n", EH->src[0], EH->src[1], EH->src[2], EH->src[3], EH->src[4], EH->src[5]);//�۽��� MAC
	printf("��  Dst MAC : %02x-%02x-%02x-%02x-%02x-%02x\n", EH->des[0], EH->des[1], EH->des[2], EH->des[3], EH->des[4], EH->des[5]);//������ MAC
	IPHeader* IH = (struct IPHeader*)(data + 14); //���� ó�� 14byte�� �̴��� ���(Layer 2) �� ������ IP���(20byte), �� ������ TCP ���...
	CheckSummer* CS = (struct CheckSummer*)(data + 14); //üũ���� ���� �� ����
	//���� ������ 01010101�̹Ƿ� ������ �ڸ���� ���ص� ��.
	//����� �ٴ� Layer2�� �����͸�ũ �������� �ڸ��� ��.

	printf("Src Port Num : %d\n", ntohs(IH->source_port));
	printf("Dest Port Num : %d\n", ntohs(IH->dest_port));


	if (type == IPHEADER)
	{
		int partSum = ntohs(CS->part1) + ntohs(CS->part2) + ntohs(CS->part3) + ntohs(CS->part4) + ntohs(CS->part5) + ntohs(CS->part6) + ntohs(CS->part7) + ntohs(CS->part8) + ntohs(CS->part9);
		u_short Bit = partSum >> 16;
	//	printf("��Ʈ �� : %08x\n", partSum);
	//	printf("4ĭ �̵� : %08x\n", Bit);
		partSum = partSum - (Bit * 65536);
	//	printf("�ѱ�� ���� ��Ʈ �� : %04x\n", partSum + Bit);
	//	printf("���� ���ϱ� : %04x\n", (u_short)~(partSum + Bit));
	//	printf("üũ�� : %04x\n", ntohs(CS->checksum));
	//	if (ntohs(CS->checksum) == (u_short)~(partSum + Bit))
	//		printf("�ջ���� ���� ���� ��Ŷ�Դϴ�.\n");
	//	else
	//		printf("�ջ�� ��Ŷ�Դϴ�. �� ���� ��û�� �ؾ� �մϴ�.\n");
		printf("���� : IPv%d\n", IH->Version);
		printf("��� ���� : %d\n", (IH->HeaderLength) * 4);
	//	printf("���� ���� : %04x\n", IH->TypeOfService);
		printf("��ü ũ�� : %d\n", ntohs(IH->HeaderLength));//2 bytes �̻� ���ʹ� ������ ������� �ϹǷ� ntohs�Լ��� �Ἥ �����´�.
		printf("��Ŷ ID : %d\n", ntohs(IH->ID));
	//	if (0x4000 == ((ntohs(IH->FlagOffset)) & 0x4000))
	//		printf("[1] ����ȭ ���� ���� ��Ŷ�Դϴ�.\n");
	//	else
	//		printf("[0] ���� ����ȭ�� ��Ŷ\n");
	//	if (0x2000 == ((ntohs(IH->FlagOffset)) & 0x2000))
	//		printf("[1] ����ȭ�� ��Ŷ�� �� �ֽ��ϴ�.\n");
	//	else
	//		printf("[0] ������ ��Ŷ�Դϴ�.\n");
	//	printf("�����׸�Ʈ ������ : %d[byte]\n", (0x1FFF & ntohs(IH->FlagOffset) * 8));
		printf("TTL : %d\n", IH->TimeToLive);
		printf("�������� : "/*, IH->Protocol*/);
		switch (IH->Protocol)
		{
		case IPPROTO_ICMP:
			printf("Internet Control Message\n");
			break;
		case IPPROTO_IGMP:
			printf("Internet Group Management\n");
			break;
		case IPPROTO_TCP:
			printf("TCP\n");
			break;
		case IPPROTO_PUP:
			printf("PUP\n");
			break;
		case IPPROTO_UDP:
			printf("UDP\n");
			break;
		case IPPROTO_IDP:
			printf("XNS IDP\n");
			break;
		case IPPROTO_PIM:
			printf("Independent Multicast\n");
			break;
		case IPPROTO_RAW:
			printf("Raw IP Packets\n");
			break;
		default:
			printf("Unknown\n");
		}
		printf("üũ�� : %04x\n", ntohs(IH->checksum));//��) 0x145F
		printf("��� IP �ּ� : %d.%d.%d.%d\n", IH->SenderAddress.ip1, IH->SenderAddress.ip2, IH->SenderAddress.ip3, IH->SenderAddress.ip4);
		printf("���� IP �ּ� : %d.%d.%d.%d\n", IH->DestinationAddress.ip1, IH->DestinationAddress.ip2, IH->DestinationAddress.ip3, IH->DestinationAddress.ip4);
	//	printf("�ɼ�/�е� : %d\n", IH->Option_Padding);
	//	printf("��Protocol : IP\n");

		
	}
	else if (type == ARPHEADER)
	{
		printf("��Protocol : ARP\n");
	}
	else if (type == RARPHEADER)
		printf("��Protocol : RARP\n");
	printf("����������������������������������������������������\n");
}
