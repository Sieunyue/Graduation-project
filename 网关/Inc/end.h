#ifndef __END_H__
#define __END_H__


typedef struct
{
    uint8_t cool_bright;
    uint8_t warm_bringt;
    uint8_t on_state;
    uint8_t bright;
    uint8_t proportion;
    uint8_t mac[8];
    //uint8_t *short_addr;
}END_TypeDef;
typedef struct 
{
    int end_num;
    uint8_t *save_end[10];
}END_Head;

void PushCmdToEnd(END_TypeDef *mend,uint8_t cmd_type, int val1, int val2);
void EndMsg(uint8_t *recbuf);
void CreatNewDevice(uint8_t *recbuf);

#endif 
