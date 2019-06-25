#ifndef _SMART_AMP_H_
#define _SMART_AMP_H_

#include <linux/types.h>
#include <sound/apr_audio-v2.h>
#include <linux/delay.h>

#define SMART_AMP

/* Below 3 should be same as in aDSP code */
#define AFE_PARAM_ID_SMARTAMP_DEFAULT		0x10001166
#define AFE_SMARTAMP_MODULE		0x10001161
#define MAX_DSP_PARAM_INDEX		500

#define RX_PORT_ID 0x1016
#define TX_PORT_ID 0x1017

#define AFE_SA_SET_PROFILE	3817
#define AFE_SA_SET_RE		3818


#define TAS_GET_PARAM			1
#define TAS_SET_PARAM			0
#define TAS_PAYLOAD_SIZE		14

#define SLAVE1		0x98
#define SLAVE2		0x9A
#define SLAVE3		0x9C
#define SLAVE4		0x9E

#define AFE_SA_IS_SPL_IDX(X)	((((X) >= 3810) && ((X) < 3899)) ? 1 : 0)
struct afe_smartamp_set_params_t {
    uint32_t payload[TAS_PAYLOAD_SIZE];
} __packed;

struct afe_smartamp_config_command {
	struct apr_hdr                      hdr;
	struct afe_port_cmd_set_param_v2    param;
	struct afe_port_param_data_v2       pdata;
	struct afe_smartamp_set_params_t	prot_config;
} __packed;

struct afe_smartamp_get_params_t {
    uint32_t payload[TAS_PAYLOAD_SIZE];
} __packed;

struct afe_smartamp_get_calib {
	struct apr_hdr 						hdr;
	struct afe_port_cmd_get_param_v2	get_param;
	struct afe_port_param_data_v2		pdata;
	struct afe_smartamp_get_params_t	res_cfg;
} __packed;

struct afe_smartamp_calib_get_resp {
	uint32_t status;
	struct afe_port_param_data_v2 pdata;
	struct afe_smartamp_get_params_t res_cfg;
} __packed;

int afe_smartamp_algo_ctrl(u8 *user_data, uint32_t param_id,
			uint8_t get_set, int32_t length);
#endif
