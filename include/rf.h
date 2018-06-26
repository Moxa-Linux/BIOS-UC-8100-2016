#ifndef _RF_H_
#define _RF_H_

struct _rf_info
{
	unsigned short vendor;
	unsigned short product;
	unsigned char out_bulk;
	unsigned char in_bulk;
	unsigned char gps_out_bulk;
	unsigned char gps_in_bulk;
	char modelName[32];
}__attribute__((packed));
#endif /*_RF_H_ */
