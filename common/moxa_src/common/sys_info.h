
#ifndef __MOXA_INFO_H__
#define __MOXA_INFO_H__

typedef struct board_info {
	char		modeltype;
	char		modelname[20];
	char		dtbname[32];
	unsigned int	sys_mmc;
	unsigned int	ext_mmc1;
	unsigned int	ext_mmc2;
	unsigned int 	mmc0;
	unsigned int 	mmc1;
	unsigned int 	mmc2;
	unsigned int 	usb_num;
	unsigned int 	usb_dongle;
	unsigned int 	usb_cellular;
} board_infos;

enum {
	UNUSED = -1,
	USED = 1,
};

int board_info_init(board_infos *board_info);

#endif
