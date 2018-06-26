#ifndef _GPS_H_
#define _GPS_H_

struct gps_gpgga_info {
	char gps_utc[32];
	char gps_lat[32];
	char gps_lat_ns[32];
	char gps_lon[32];
	char gps_lon_ns[32];
	char gps_qi[32];
	char gps_num_of_sat[32];
	char gps_hdop[32];
	char gps_mean_sea[32];
	char gps_unit_of_an_al[32];
	char gps_geo_sep[32];
	char gps_unit_of_ge_se[32];
	char gps_age_in_sec[32];
	char gps_id[32];
};

#endif /*_GPS_H_ */
