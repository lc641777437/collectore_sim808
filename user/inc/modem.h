/*
 * modem.h
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#ifndef USER_INC_MODEM_H_
#define USER_INC_MODEM_H_

eat_bool modem_cmd(const unsigned char *cmd);


eat_bool modem_IsCallReady(char* modem_rsp);

eat_bool modem_GPRSAttach(void);

eat_bool modem_switchEngineeringMode(int mode, int Ncell);

eat_bool modem_readCellInfo(void);

eat_bool modem_readCCIDInfo(void);
eat_bool modem_IsCCIDOK(char* modem_rsp);

eat_bool modem_GNSS(void);

eat_bool modem_AT(unsigned char *cmd);



#endif /* USER_INC_MODEM_H_ */
