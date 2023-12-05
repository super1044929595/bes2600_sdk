#ifndef NOSTD

#include <math.h>
#include <stdio.h>

#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "heap_api.h"

#include "string.h"
#include "besbt_string.h"
#include "bt_drv.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_btdump.h"
#include "iqcorrect.h"
#include "nvrecord.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"

#include "bt_drv_iq_common.h"

#ifdef TX_IQ_CAL

#define DCCalib
#define DC_OFFSET_MAX   32

//#define IQ_CALI_DEBUG
//#define IQ_CALI_PRINT

#define IQ_ENERGY_THD   300

#define LOG_MODULE                          HAL_TRACE_MODULE_APP

#define NTbl (1024)
#define fftSize 1024

#define bitshift 10
#define pi (3.1415926535898)
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define max(a,b)    (((a) > (b)) ? (a) : (b))

#define READ_REG(b,a) *(volatile uint32_t *)(uintptr_t)((uint32_t)(b)+(a))
#define WRITE_REG(v,b,a) *(volatile uint32_t *)(uintptr_t)((uint32_t)(b)+(a)) = v

#define   BUF_SIZE   (1024)

#define MAX_COUNT 5
volatile int iqimb_dma_status = 0;
//short M0data[BUF_SIZE];
#define MED_MEM_POOL_SIZE (65*1024)
static uint8_t *g_medMemPool = NULL;

#ifdef DCCalib
static int g_dc_i_val = 0;
static int g_dc_q_val = 0;
#endif

extern void *rt_malloc(unsigned int size);
extern void rt_free(void *rmem);

typedef struct ComplexInt_
{
    int re;
    int im;
} ComplexInt;
typedef struct Complexflt_
{
    float re;
    float im;
} ComplexFlt;
typedef struct ComplexShort_
{
    short re;
    short im;
} ComplexShort;

#ifdef DCCalib
static void Tblgen(ComplexShort*w0, ComplexShort*w1, ComplexShort*w2,int len)
{
    for (int i=0; i<len; i++)
    {
        int offset = 1;
        w0[i].re = (short)(cos(2*pi*i/(2048.0/(32+offset)))*32767);
        w0[i].im = (short)(sin(2*pi*i/(2048.0/(32+offset)))*32767);
        w1[i].re = (short)(cos(2*pi*i/(2048.0/(31+offset)))*32767);
        w1[i].im = (short)(sin(2*pi*i/(2048.0/(31+offset)))*32767);
        w2[i].re = (short)(cos(2*pi*i/(2048.0/(33+offset)))*32767);
        w2[i].im = (short)(sin(2*pi*i/(2048.0/(33+offset)))*32767);
    }
}
#endif
static void Tblgen_iq(ComplexShort*w0, ComplexShort*w1, ComplexShort*w2,int len)
{
    for (int i=0; i<len; i++)
    {

        w0[i].re = (short)(cos(2*pi*i/(2048.0/66))*32767);
        w0[i].im = (short)(sin(2*pi*i/(2048.0/66))*32767);
        w1[i].re = (short)(cos(2*pi*i/(2048.0/65))*32767);
        w1[i].im = (short)(sin(2*pi*i/(2048.0/65))*32767);
        w2[i].re = (short)(cos(2*pi*i/(2048.0/67))*32767);
        w2[i].im = (short)(sin(2*pi*i/(2048.0/67))*32767);
    }
}
/*
typedef struct IQMismatchPreprocessState_
{
    short *M0data;
    ComplexShort *Table0;
    ComplexShort *Table1;
    ComplexShort *Table2;
} IQMismatchPreprocessState;


IQMismatchPreprocessState st_g;
short st_M0data[fftSize];
short st_Table0[2*NTbl];
short st_Table1[2*NTbl];
short st_Table2[2*NTbl];

IQMismatchPreprocessState *st;


IQMismatchPreprocessState *IQMismatchPreprocessState_init(int Cohcnt)
{
    st_g.M0data = st_M0data;
    st_g.Table0= st_Table0;
    st_g.Table1= st_Table1;
    st_g.Table2= st_Table2;
    return &st_g;
}
*/

typedef struct IQMismatchPreprocessState_
{
    short *M0data;
    ComplexShort *Table0;
    ComplexShort *Table1;
    ComplexShort *Table2;
} IQMismatchPreprocessState;

typedef struct IQPreprocessState_rx
{
    short *M0data;
    int *data_i;
    int *data_q;
} IQState_rx;

IQMismatchPreprocessState *IQMismatchPreprocessState_init(int fft_size)
{
    BT_DRV_TRACE(0,"malloc ini");
    IQMismatchPreprocessState *st = (IQMismatchPreprocessState *)med_calloc(1, sizeof(IQMismatchPreprocessState));
    if (st){
        st->M0data = (short*)med_calloc(fft_size*2, sizeof(short));
        st->Table0 = (ComplexShort*)med_calloc(fft_size, sizeof(ComplexShort));
        st->Table1 = (ComplexShort*)med_calloc(fft_size, sizeof(ComplexShort));
        st->Table2 = (ComplexShort*)med_calloc(fft_size, sizeof(ComplexShort));
        BT_DRV_TRACE(5,"st:%p, st->M0data:%p, st->Table0:%p, st->Table1:%p, st->Table2:%p", st, st->M0data, st->Table0, st->Table1, st->Table2);
        BT_DRV_TRACE(0,"malloc ok");
    }
    return st;
}

IQState_rx *IQState_init_rx(int fft_size)
{
    BT_DRV_TRACE(1,"%s malloc ini", __func__);
    IQState_rx *st = (IQState_rx *)med_calloc(1, sizeof(IQState_rx));
    if (st){
        st->M0data = (short*)med_calloc(fft_size, sizeof(short));
        st->data_i = (int*)med_calloc(fft_size/2, sizeof(int));
        st->data_q = (int*)med_calloc(fft_size/2, sizeof(int));

        BT_DRV_TRACE(1,"%s malloc ok", __func__);
    }
    return st;
}

int32_t IQState_init_rx_destroy(IQState_rx *st)
{
    med_free(st->M0data);
    med_free(st->data_i);
    med_free(st->data_q);
    med_free(st);
    return 0;
}

int32_t IQMismatchPreprocessState_destroy(IQMismatchPreprocessState *st)
{
    BT_DRV_TRACE(5,"st:%p, st->M0data:%p, st->Table0:%p, st->Table1:%p, st->Table2:%p", st, st->M0data, st->Table0, st->Table1, st->Table2);
    med_free(st->M0data);
    med_free(st->Table0);
    med_free(st->Table1);
    med_free(st->Table2);
    med_free(st);
    return 0;
}

int IQMismatchParameterCalc_ex(const short *M0data,
                               short Cohcnt, ComplexShort *Table0, ComplexShort *Table1, ComplexShort *Table2,int fftsize)
{
    int i,j,k;
    float M0 = 0;
    //ComplexInt tmp0;
    ComplexFlt tmp0;
    ComplexFlt tmp1;
    ComplexFlt tmp2;
    for (j = 0; j < Cohcnt; j++)
    {
        tmp0.re = 0.0f;
        tmp0.im = 0.0f;
        tmp1.re = 0.0f;
        tmp1.im = 0.0f;
        tmp2.re = 0.0f;
        tmp2.im = 0.0f;
        k = 0;
        for (i=0; i<fftsize; i++)
        {
            if(k>=fftsize)
                k = 0;
            tmp0.re = tmp0.re + (((int)M0data[2*i] *(int)Table0[k].re) >> 15) ;
            tmp0.im = tmp0.im + (((int)M0data[2*i] * (int)Table0[k].im) >> 15) ;
            tmp1.re = tmp1.re + (((int)M0data[2*i] *(int)Table1[k].re) >> 15) ;
            tmp1.im = tmp1.im + (((int)M0data[2*i] * (int)Table1[k].im) >> 15) ;
            tmp2.re = tmp2.re + (((int)M0data[2*i] *(int)Table2[k].re) >> 15) ;
            tmp2.im = tmp2.im + (((int)M0data[2*i] * (int)Table2[k].im) >> 15) ;
            k = k + 1;
        }
        tmp0.re = tmp0.re/ fftsize;// >> bitshift;
        tmp0.im = tmp0.im/ fftsize;// >>bitshift;
        tmp1.re = tmp1.re/ fftsize;// >> bitshift;
        tmp1.im = tmp1.im/ fftsize;// >>bitshift;
        tmp2.re = tmp2.re/ fftsize;// >> bitshift;
        tmp2.im = tmp2.im/ fftsize;// >>bitshift;
        M0 = M0 + tmp0.re*tmp0.re + tmp0.im*tmp0.im;
        M0 = M0 + tmp1.re*tmp1.re + tmp1.im*tmp1.im;
        M0 = M0 + tmp2.re*tmp2.re + tmp2.im*tmp2.im;
    }
    return (int)(M0/Cohcnt);
}


void caculate_energy_main_test(IQMismatchPreprocessState *st,int* Energy,int* Energy1,int fftsize)
{
    short Cohcnt = 1;
    *Energy1 = IQMismatchParameterCalc_ex(st->M0data, Cohcnt, st->Table0, st->Table1, st->Table2,fftsize);
}

static struct HAL_DMA_DESC_T iqimb_dma_desc[1];
static uint8_t g_dma_channel = HAL_DMA_CHAN_NONE;

static void iqimb_dma_dout_handler(uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    if(g_dma_channel != HAL_DMA_CHAN_NONE)
    {
        hal_audma_free_chan(g_dma_channel);
    }
    hal_btdump_disable();
    iqimb_dma_status = 0;

    return;
}

int bt_iqimb_dma_enable ( short * dma_dst_data, uint16_t size)
{
    int sRet = 0;
    struct HAL_DMA_CH_CFG_T iqimb_dma_cfg;
    // BT_DRV_TRACE(0,"bt_iqimb_dma_enable :");

    iqimb_dma_status = 1;

    sRet = memset_s(&iqimb_dma_cfg, sizeof(iqimb_dma_cfg), 0, sizeof(iqimb_dma_cfg));
    if (sRet){
        BT_DRV_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }

    //iqimb_dma_cfg.ch = hal_audma_get_chan(HAL_AUDMA_DSD_RX,HAL_DMA_HIGH_PRIO);

    iqimb_dma_cfg.ch = hal_audma_get_chan(HAL_AUDMA_BTDUMP,HAL_DMA_HIGH_PRIO);
    ASSERT((HAL_DMA_CHAN_NONE != iqimb_dma_cfg.ch), "hal_audma_get_chan failed.");
    g_dma_channel = iqimb_dma_cfg.ch;           //backup dma channel for interrupt free

    iqimb_dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    iqimb_dma_cfg.dst_periph = 0; //useless
    iqimb_dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    iqimb_dma_cfg.handler = (HAL_DMA_IRQ_HANDLER_T)iqimb_dma_dout_handler;
    iqimb_dma_cfg.src = 0; // useless
    iqimb_dma_cfg.src_bsize = HAL_DMA_BSIZE_4;

    //iqimb_dma_cfg.src_periph = HAL_AUDMA_DSD_RX;
    iqimb_dma_cfg.src_periph = HAL_AUDMA_BTDUMP;
    iqimb_dma_cfg.src_tsize = size;//1600; //1600*2/26=123us
    iqimb_dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    //iqimb_dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    iqimb_dma_cfg.try_burst = 1;
    iqimb_dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    iqimb_dma_cfg.dst = (uintptr_t)(dma_dst_data);

    hal_audma_init_desc(&iqimb_dma_desc[0], &iqimb_dma_cfg, 0, 1);

    hal_audma_sg_start(&iqimb_dma_desc[0], &iqimb_dma_cfg);

    //configed after mismatch parameter done, or apb clock muxed.

    //wait
    for(volatile int i=0; i<5000; i++);
    hal_btdump_enable();

    return 1;

}

int bt_iqimb_ini ()
{
    uint32_t val;

    BT_DRV_TRACE(0,"bt_iqimb_ini :");
    //config gpadc 26m
    val = READ_REG(0xd0350208,0x0);
    val &= 0xffefffff; //bit20
    val |= (1<<18); //reg_gclk_en(BTM_CLK_26RXNFMI)
    val &= 0xffbfffff; //bit22
    val |= (1<<20); //reg_gclk_en(BTM_CLK_26RXDUMP)
    val &= 0xfdffffff; //bit25
    val |= (1<<25); //reg_gclk_en(BTM_CLK_26RXADC)
    WRITE_REG(val,0xd0350208,0x0);
    BT_DRV_TRACE(1,"0xd0350208 : 0x%08x",val);

    val = READ_REG(0xd0350228,0x0);
    val &= 0xffefffff; //bit20
    val |= (1<<18); //reg_gclk_mode(BTM_CLK_26RXNFMI)
    val &= 0xffbfffff; //bit22
    val |= (1<<20); //reg_gclk_mode(BTM_CLK_26RXDUMP)
    val &= 0xfdffffff; //bit25
    val |= (1<<25); //reg_gclk_mode(BTM_CLK_26RXADC)
    WRITE_REG(val,0xd0350228,0x0);
    BT_DRV_TRACE(1,"0xd0350228 : 0x%08x",val);

    //for rx mux to gpadc
    val = READ_REG(0xd0350218,0x0);
    val &= 0xfffffff0; //bit3:0 1110 reg_diagcntl(3 downto 0)
    val |= (0x4<<0);//iq_cali

    val &= 0xfffffbff; //bit10 1  reg_diagcntl(10)
    val |= (0x1<<10);
    WRITE_REG(val,0xd0350218,0x0);

    BT_DRV_TRACE(1,"0xd0350218 : 0x%08x",val);

    //No need to set timer_wt26 on 1305
/*     val = READ_REG(0xd0330024,0x0);
    val &= 0xfffffff7; //bit3:0 1110 reg_diagcntl(3 downto 0)
    val |= (0x1<<15);
    WRITE_REG(val,0xd0330024,0x0);
    BT_DRV_TRACE(1,"0xd0330024 : 0x%08x",val); */


    val = READ_REG(0xd0330034,0x0);
    val &= 0x03ffffff; //bit3:0 1110 reg_diagcntl(3 downto 0)
    val |= (0x1b<<26);
    WRITE_REG(val,0xd0330034,0x0);

    BT_DRV_TRACE(1,"0xd0330034 : 0x%08x",val);

    WRITE_REG(0x002eb948,0xd0350364,0x0);


    val = READ_REG(0xd0330038,0x0);
    val |= (0x1<<24);
    val &= 0xffcfffff;
    val |= (0x3<<20);
    WRITE_REG(val,0xd0330038,0x0);

    WRITE_REG(0,0xc000033c,0x0);
    return 1;
}
void check_mem_data(void* data, int len)
{
    short* share_mem = (short*)data;
    BT_DRV_TRACE(3,"check_mem_data :share_mem= %p, 0x%x, 0x%x",share_mem,share_mem[0],share_mem[1]);

    int32_t i =0;
    int32_t remain = len;

    while(remain > 0)
    {
        for(i=0; i<32; i++)//output two line
        {
            if (remain >16)
            {
                DUMP16("%04X ",share_mem,16);
                share_mem +=16;
                remain -= 16;
            }
            else
            {
                DUMP16("%04X ",share_mem,remain);
                remain =0;
                return;
            }
        }
        //  BT_DRV_TRACE(0,"\n");
        //BT_DRV_TRACE(1,"addr :0x%08x\n",share_mem);
        hal_sys_timer_delay(MS_TO_TICKS(100));
    }
}

int bt_Txdc_cal_set(int ch_num, int dc_add)
{
    uint32_t val;
    uint32_t tmp = (uint32_t)dc_add;
    //sel apb clock
    val = READ_REG(0xd0350348,0x0);
    if(ch_num==0)
    {
        val &= 0xfffffc00; //bit31 1 int_en_mismatch
        val |= tmp & 0x3ff;
    }
    else
    {
        // BT_DRV_TRACE(1,"bt_Txdc_cal_set, dcadd : 0x%08x",tmp);
        val &= 0xfc00ffff; //bit31 1 int_en_mismatch
        val |= (tmp & 0x3ff) <<16;
    }
    WRITE_REG(val,0xd0350348,0x0);


    // BT_DRV_TRACE(1,"bt_Txdc_cal_set, 0xd0350348 : 0x%08x",val);

    return 1;
}

//g/p mismatch base addr 0xd0310000
//dc_i: 0xd0350348 9:0
//dc_q: 0xd0350348 25:16
int bt_iqimb_add_mismatch(int ch_num, int gain_mis, int phase_mis, int dc_i, int dc_q, uint32_t addr)
{
    uint32_t val;
    //sel apb clock
    val = READ_REG(0xd0350348,0x0);
    val &= 0x7fffffff; //bit31 1 int_en_mismatch
    val |= (0x0<<31);
    WRITE_REG(val,0xd0350348,0x0);

    val = (phase_mis << 16) | (gain_mis & 0x0000ffff);
    WRITE_REG(val,addr,ch_num*4);
    //tval = READ_REG(addr,0x0);
    //BT_DRV_TRACE(1,"bt_iqimb_add_mismatch, iq : 0x%08x",tval);

    WRITE_REG(0x400000,0xd0350220,0);
    WRITE_REG(0x400000,0xd0350224,0);
    // bt_Txdc_cal_set(0,dc_i);
    //bt_Txdc_cal_set(1,dc_q);

    /*
        val = READ_REG(0xd0350348,0x0);
        val &= 0xfffffc00; //bit9:0
        val |= dc_i;
        val &= 0xfc00ffff; //bit25:16
        val |= (dc_q<<16);
        WRITE_REG(val,0xd0350348,0x0);
    */
    //sel 26m clock
    val = READ_REG(0xd0350348,0x0);
    val &= 0x7fffffff; //bit31 1 int_en_mismatch
    val |= (0x1<<31);
    WRITE_REG(val,0xd0350348,0x0);

    return 1;
}

void DC_correction(IQMismatchPreprocessState *st,int *dc_i_r,int *dc_q_r,int fftsize,int i_min_val, int q_min_val, int dc_min_energy)
{
    uint8_t k;
    int Energy,Energy1;
    double tmp;
    int dc_iters = 4;
    int dc_i;
    int dc_q;
    int dc_step = 4;
    int dc_i_base;
    int dc_q_base;
    int P0,PIplus,PIneg,PQplus,PQneg;
    int dc_i_min = i_min_val;
    int dc_q_min = q_min_val;
    int energy_min = dc_min_energy;

    dc_i_base =i_min_val;
    dc_q_base = q_min_val;
    for(k=0; k<dc_iters; k++)
    {
        dc_i = dc_i_base;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        P0 = Energy1;
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(0,"k=%d,dc_i=%d,dc_q=%d,P0=%d",k,dc_i,dc_q,Energy1);
#endif
        if(energy_min > Energy1)
        {
            dc_i_min = dc_i;
            dc_q_min = dc_q;
        }
        dc_i = dc_i_base + dc_step;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PIplus = Energy1;
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(0,"k=%d,dc_i=%d,dc_q=%d,PIplus=%d",k,dc_i,dc_q,Energy1);
#endif
        if(energy_min > Energy1)
        {
            dc_i_min = dc_i;
            dc_q_min = dc_q;
            energy_min = Energy1;
        }

        dc_i = dc_i_base - dc_step;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PIneg = Energy1;
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(0,"k=%d,dc_i=%d,dc_q=%d,PIneg=%d",k,dc_i,dc_q,Energy1);
#endif
        if(energy_min > Energy1)
        {
            dc_i_min = dc_i;
            dc_q_min = dc_q;
            energy_min = Energy1;
        }

        dc_i = dc_i_base;
        dc_q = dc_q_base + dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PQplus = Energy1;
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(0,"k=%d,dc_i=%d,dc_q=%d,PQplus=%d",k,dc_i,dc_q,Energy1);
#endif
        if(energy_min > Energy1)
        {
            dc_i_min = dc_i;
            dc_q_min = dc_q;
            energy_min = Energy1;
        }

        dc_i = dc_i_base;
        dc_q = dc_q_base - dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PQneg = Energy1;
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(0,"k=%d,dc_i=%d,dc_q=%d,PQneg=%d",k,dc_i,dc_q,Energy1);
#endif
        if(energy_min > Energy1)
        {
            dc_i_min = dc_i;
            dc_q_min = dc_q;
            energy_min = Energy1;
        }
#if 0
        dc_i = dc_i_base + dc_step;
        dc_q = dc_q_base + dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        BT_DRV_TRACE(3,"dc_i=%d,dc_q=%d,Energy1=%d",dc_i,dc_q,Energy1);
        PIPQ = Energy1;
        BT_DRV_TRACE(0,"k=%d,dc_i=%d,dc_q=%d,PIPQ=%d",k,dc_i,dc_q,Energy1);
        tmp = 2*PIplus*PIplus + 2*PQplus*PQplus + 2*PIPQ*PIPQ;
        BT_DRV_TRACE(0,"1tmp=%d",(int)tmp);
        tmp = tmp - 6*P0*P0;
        BT_DRV_TRACE(0,"2tmp=%d",(int)tmp);
        tmp = tmp + 4*P0*(PIneg+PQneg+PIPQ);
        BT_DRV_TRACE(0,"3tmp=%d",(int)tmp);
        tmp = tmp + 2*(PIplus*(PQplus - PQneg)-PIneg*(PQplus+PQneg));
        BT_DRV_TRACE(0,"4tmp=%d",(int)tmp);
        tmp_D = tmp - 4*PIPQ*(PIplus+PQplus);
        BT_DRV_TRACE(0,"5tmp=%d",(int)tmp);
        tmp = P0*(2*(PIplus - PIneg)+PQplus -PQneg);
        BT_DRV_TRACE(0,"6tmp=%d",(int)tmp);
        tmp = tmp - PIPQ*(PQneg-PQplus)+PIneg*(PQplus+PQneg);
        BT_DRV_TRACE(0,"7tmp=%d",(int)tmp);
        tmp = tmp + PQplus*(PQneg-PQplus-2*PIplus);
        BT_DRV_TRACE(0,"8tmp=%d",(int)tmp);
        CI = -dc_step*tmp;
        tmp = P0*(2*(PQplus - PQneg)+PIplus-PIneg);
        BT_DRV_TRACE(0,"9tmp=%d",(int)tmp);
        tmp = tmp - PIPQ*(PIneg-PIplus)+PQneg*(PIplus+PIneg);
        BT_DRV_TRACE(0,"10tmp=%d",(int)tmp);
        tmp = tmp + PIplus*(PIneg-PIplus-2*PQplus);
        BT_DRV_TRACE(0,"11tmp=%d",(int)tmp);
        CQ = -dc_step*tmp;
        BT_DRV_TRACE(0,"CI=%d,CQ=%d",(int)CI,(int)CQ);
        if(0==tmp_D)
        {
            BT_DRV_TRACE(0,"!!!tmp_D = 0!!!");
        }
        else
        {
            dc_i_base = dc_i_base + (int)(CI/tmp_D);
            dc_q_base = dc_q_base + (int)(CQ/tmp_D);
        }

#else
        tmp = PIplus -2*P0 + PIneg;
        tmp = (PIplus-PIneg)*dc_step/tmp;
        tmp = min(tmp,2*dc_step);
        tmp = max(tmp,-2*dc_step);
        //BT_DRV_TRACE(0,"1tmp=%d",(int)tmp);
        dc_i_base = dc_i_base - (int)(tmp);

        tmp = PQplus -2*P0 + PQneg;
        tmp = (PQplus-PQneg)*dc_step/tmp;
        tmp = min(tmp,2*dc_step);
        tmp = max(tmp,-2*dc_step);
        //BT_DRV_TRACE(0,"1tmp=%d",(int)tmp);
        dc_q_base = dc_q_base - (int)(tmp);

#endif
        bt_Txdc_cal_set(0,dc_i_min);
        bt_Txdc_cal_set(1,dc_q_min);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_min = Energy1;
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(3,"dc_i_min=%d,dc_q_min=%d,energy_min=%d",dc_i_min,dc_q_min,Energy1);
#endif
        bt_Txdc_cal_set(0,dc_i_base);
        bt_Txdc_cal_set(1,dc_q_base);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(3,"dc_i_base=%d,dc_q_base=%d,Energy = %d",dc_i_base,dc_q_base,Energy1);
#endif
        if(energy_min < Energy1)
        {
            dc_i_base = dc_i_min;
            dc_q_base = dc_q_min;
            energy_min = Energy1;
        }
        if(200>energy_min)
        {
            dc_iters = 1;
        }
    }
    *dc_i_r = dc_i_base;
    *dc_q_r = dc_q_base;
}
static int IQ_GAIN_Mismatch_Correction(IQMismatchPreprocessState *st,int gain_min_val,int phase_mis_base,int fftsize,uint32_t addr)
{
    uint8_t k = 0;
    int phase_mis_tmp =0;
    int gain_mis_tmp =0;
    int tmp = 0;
    int energy_ret0_last = 1048576;
    int energy_ret0 = 0;
    int energy_ret1 = 0;
    int energy_ret2 = 0;
    int Energy,Energy1 ;
    int iters = 4;
    int gainstep =8;
    int gain_mis_base = gain_min_val;

    phase_mis_tmp = phase_mis_base;
    energy_ret0_last = 1048576;
    for(k=0; k<iters+2; k++)
    {
        gain_mis_tmp = gain_mis_base;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret0 = Energy1;
        if(energy_ret0<50)
            break;
        if(k>0)
        {
            if(energy_ret0_last < energy_ret0)
            {
                gain_mis_base = gain_mis_base + tmp;
                gain_mis_tmp = gain_mis_base;
                bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                energy_ret0 = Energy1;
            }
        }
        //BT_DRV_TRACE(1,"IQ gain correct energy_ret0 = %d",energy_ret0);
        gainstep = 4;///////////////////
        gain_mis_tmp = gain_mis_base + gainstep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret2 = Energy1;
        gain_mis_tmp = gain_mis_base - gainstep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret1 = Energy1;
        tmp = energy_ret2 -2*energy_ret0 + energy_ret1;
        //BT_DRV_TRACE(6,"E(%d)=%d,E(%d)=%d,E(%d)=%d",
        //gain_mis_base - gainstep,energy_ret1,gain_mis_base,energy_ret0,gain_mis_base + gainstep,energy_ret2);
        //BT_DRV_TRACE(1,"IQ gain correct energy_ret2 -2*energy_ret0 + energy_ret1 = %d",tmp);
        if(tmp>0)
        {
            tmp = (energy_ret2-energy_ret1)*gainstep/tmp/2;
            tmp = min(tmp,4*gainstep);
            gainstep = gainstep/2;

            if(0==tmp)
            {
                if(energy_ret1<energy_ret0)
                {
                    tmp = gainstep;
                }
                if(energy_ret2<energy_ret0 && energy_ret2<energy_ret1)
                {
                    tmp = -gainstep;
                }
                //BT_DRV_TRACE(1,"0 tmp=%d",tmp);
            }
            //BT_DRV_TRACE(1,"1 tmp=%d",tmp);
        }
        else
        {
            tmp = 0;
            iters = iters+1;
            if(energy_ret1<energy_ret0)
            {
                tmp = gainstep;
            }
            if(energy_ret2<energy_ret0 && energy_ret2<energy_ret1)
            {
                tmp = -gainstep;
            }
            //BT_DRV_TRACE(1,"2 tmp=%d",tmp);
        }
        if(iters>8)
            break;
        if((gain_mis_base - tmp>120)||(gain_mis_base - tmp<-120))
            tmp = 0;
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(2,"IQ gain correct gain_mis = %d ,gain_adj = %d",gain_mis_base,tmp);
#endif
        gain_mis_base = gain_mis_base - tmp;
        energy_ret0_last = energy_ret0;
    }
    return gain_mis_base;
}
static int IQ_Phase_Mismatch_Correction(IQMismatchPreprocessState *st,int phase_min_val,int gain_mis_base,int fftsize,uint32_t addr)
{
    uint8_t k = 0;
    int phase_mis_tmp =0;
    int gain_mis_tmp =0;
    int tmp = 0;
    int energy_ret0_last = 1048576;
    int energy_ret0 = 0;
    int energy_ret1 = 0;
    int energy_ret2 = 0;
    int Energy,Energy1 ;
    int iters = 4;
    int phasestep =4;
    int phase_mis_base = phase_min_val;
    tmp = 0;
    energy_ret0_last = 1048576;
    gain_mis_tmp = gain_mis_base;
    for(k=0; k<iters; k++)
    {
        phase_mis_tmp = phase_mis_base;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret0 = Energy1;
        if(k>0)
        {
            if(energy_ret0_last < energy_ret0)
            {
                phase_mis_base = phase_mis_base + tmp;
                phase_mis_tmp = phase_mis_base;
                bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                energy_ret0 = Energy1;
            }
        }
        if(energy_ret0<50)
            break;
        // BT_DRV_TRACE(1,"IQ phase correct energy_ret0 = %d",energy_ret0);
        phasestep = 4;//////////////////////////
        phase_mis_tmp = phase_mis_base + phasestep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret2 = Energy1;
        // BT_DRV_TRACE(1,"IQ phase correct energy_ret2 = %d",energy_ret2);
        phase_mis_tmp = phase_mis_base - phasestep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret1 = Energy1;
        //BT_DRV_TRACE(1,"IQ phase correct energy_ret1 = %d",energy_ret1);
        tmp = energy_ret2 -2*energy_ret0 + energy_ret1;
        // BT_DRV_TRACE(1,"IQ phase correct energy_ret2 -2*energy_ret0 + energy_ret1 = %d",tmp);
        if(tmp>0)
        {
            tmp = (energy_ret2-energy_ret1)*phasestep/tmp/2;
            tmp = min(tmp,4*phasestep);
            phasestep = phasestep/2;
        }
        else
        {
            tmp = 0;
            iters = iters + 1;
        }
        if(iters>8)
            break;
        if((phase_mis_base - tmp>60)||(phase_mis_base - tmp<-60))
            tmp = 0;
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(2,"IQ phase correct phase_mis = %d ,phase_adj = %d",phase_mis_base,tmp);
#endif
        phase_mis_base = phase_mis_base - tmp;
        energy_ret0_last = energy_ret0;

    }
    return phase_mis_base;
}

static BT_IQ_CALIBRATION_CONFIG_T config;
static void bt_update_local_iq_calibration_val(uint32_t freq_range, uint16_t gain_cal_val, uint16_t phase_cal_val)
{
    config.validityMagicNum = BT_IQ_VALID_MAGIC_NUM;
    config.gain_cal_val[freq_range] = gain_cal_val;
    config.phase_cal_val[freq_range] = phase_cal_val;
}

static BT_DC_CALIBRATION_CONFIG_T btDcCalibConfig;
static void bt_update_local_dc_calibration_val(int dc_i_val, int dc_q_val)
{
    btDcCalibConfig.validityMagicNum = BT_IQ_VALID_MAGIC_NUM;
    btDcCalibConfig.dc_i_val = dc_i_val;
    btDcCalibConfig.dc_q_val = dc_q_val;
}

// do QA calibration and get the calibration value
static POSSIBLY_UNUSED void btIqCalibration(void)
{
    btdrv_tx_iq_cal();
    BT_DRV_TRACE(0,"Acquired calibration value:");
    for (uint32_t range = 0;range < BT_FREQENCY_RANGE_NUM;range++)
    {
        BT_DRV_TRACE(3,"%d: 0x%x - 0x%x", range,
            config.gain_cal_val[range],
            config.phase_cal_val[range]);
    }
}

// get IQ calibration from NV
POSSIBLY_UNUSED static bool bt_get_iq_calibration_val_from_nv(BT_IQ_CALIBRATION_CONFIG_T* pConfig)
{
    if (nv_record_get_extension_entry_ptr() &&
        (BT_IQ_VALID_MAGIC_NUM ==
        nv_record_get_extension_entry_ptr()->btIqCalConfig.validityMagicNum))
    {
        *pConfig = nv_record_get_extension_entry_ptr()->btIqCalConfig;
        return true;
    }
    else
    {
        return false;
    }
}

// get DC calibration from NV
POSSIBLY_UNUSED static bool bt_get_dc_calibration_val_from_nv(BT_DC_CALIBRATION_CONFIG_T* pConfig)
{
    if (nv_record_get_extension_entry_ptr() &&
        (BT_IQ_VALID_MAGIC_NUM ==
        nv_record_get_extension_entry_ptr()->btDcCalibConfig.validityMagicNum)) {
        *pConfig = nv_record_get_extension_entry_ptr()->btDcCalibConfig;
        return true;
    } else {
        return false;
    }
}

static void bt_set_dc_calibration_val_to_nv(BT_DC_CALIBRATION_CONFIG_T* pConfig)
{
    uint32_t lock = nv_record_pre_write_operation();
    nv_record_get_extension_entry_ptr()->btDcCalibConfig = *pConfig;
    nv_record_post_write_operation(lock);
    nv_record_extension_update();
}

void bt_iq_calibration_setup(void)
{
    if(bt_get_iq_calibration_val_from_nv(&config) &&
       bt_get_dc_calibration_val_from_nv(&btDcCalibConfig))
    {
        BT_DRV_TRACE(0,"Calibration value in NV:");

        for (uint32_t range = 0;range < BT_FREQENCY_RANGE_NUM;range++)
        {
            TRACE_IMM(3,"%d: 0x%x - 0x%x", range,
                config.gain_cal_val[range],
                config.phase_cal_val[range]);
        }
        uint32_t addr = 0xd0310000;
        int chs = 27;

        for (uint32_t range = 0;range < 3;range++)
        {
            if (0 == range)
            {
                addr = 0xd0310034;
                chs= 27;
                addr = addr-13*4;
            }

            for (uint32_t ch=0;ch < chs;ch++)
            {
                bt_iqimb_add_mismatch(0,
                    config.gain_cal_val[range],
                    config.phase_cal_val[range],
                    0,0,addr);
                addr = addr +4;
            }
        }

        BT_DRV_TRACE(0, "DC Calib value in NV:");
        BT_DRV_TRACE(2, "dc_i=%d, dc_q=%d",btDcCalibConfig.dc_i_val, btDcCalibConfig.dc_q_val);
        g_dc_i_val = btDcCalibConfig.dc_i_val;
        g_dc_q_val = btDcCalibConfig.dc_q_val;
        bt_Txdc_cal_set(0, btDcCalibConfig.dc_i_val);
        bt_Txdc_cal_set(1, btDcCalibConfig.dc_q_val);
    }
    else
    {
        btIqCalibration();

        uint32_t lock = nv_record_pre_write_operation();
        nv_record_get_extension_entry_ptr()->btIqCalConfig = config;
        nv_record_post_write_operation(lock);

        nv_record_update_runtime_userdata();
    }
}

void bt_IQ_DC_Mismatch_Correction_Release()
{
    int phase_mis_base;
    int gain_mis_base;
    int Energy,Energy1;
    int Energy_bak = 0;
    POSSIBLY_UNUSED uint32_t time_start = hal_sys_timer_get();
    int fftsize;
    uint32_t addr = 0xd0310000;
    int chs = 27;
    IQMismatchPreprocessState *st = NULL;
    syspool_init();
    syspool_get_buff(&g_medMemPool, MED_MEM_POOL_SIZE);
    med_heap_init(&g_medMemPool[0], MED_MEM_POOL_SIZE);
    //config gpadc 26m
    READ_REG(0xd0310000,0x0);
#ifdef DCCalib
    int dc_i_base = 0;
    int dc_q_base = 0;
    int dc_i_min = 0;
    int dc_q_min = 0;
    int dc_enery_min = 2147483647;
    fftsize = 1024;
    st = IQMismatchPreprocessState_init(fftsize);
    Tblgen(st->Table0,st->Table1,st->Table2,fftsize);

    WRITE_REG(0x0,0xD02201E4,0x0);
    WRITE_REG(0x800A0041,0xD02201E4,0x0);

    for(dc_i_base=-1*DC_OFFSET_MAX; dc_i_base<=DC_OFFSET_MAX; dc_i_base+=4)
    {
        bt_Txdc_cal_set(0,dc_i_base);
        bt_Txdc_cal_set(1,0);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(3,"dc_i = %d ,dc_q = %d,Energy1 = %d",dc_i_base,dc_q_base,Energy1);
#endif
        if(dc_enery_min > Energy1)
        {
            dc_i_min = dc_i_base;
            dc_enery_min = Energy1;
        }
    }
    dc_enery_min = 2147483647;
    dc_i_base = dc_i_min;
    for(dc_q_base=-1*DC_OFFSET_MAX; dc_q_base<=DC_OFFSET_MAX; dc_q_base+=4)
    {
        bt_Txdc_cal_set(0,dc_i_base);
        bt_Txdc_cal_set(1,dc_q_base);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(3,"dc_i = %d ,dc_q = %d,Energy1 = %d",dc_i_base,dc_q_base,Energy1);
#endif
        if(dc_enery_min > Energy1)
        {
            dc_q_min = dc_q_base;
            dc_enery_min = Energy1;
        }
    }
#ifdef IQ_CALI_PRINT
    BT_DRV_TRACE(3,"dc_i_min = %d ,dc_q_min = %d,Energy1 = %d",dc_i_min,dc_q_min,dc_enery_min);
#endif

    dc_i_base = dc_i_min;
    dc_q_base = dc_q_min;
    DC_correction(st,&dc_i_base,&dc_q_base,fftsize,dc_i_min,dc_q_min,dc_enery_min);
    dc_i_min = dc_i_base;
    dc_q_min = dc_q_base;

    bt_Txdc_cal_set(0,dc_i_base);
    bt_Txdc_cal_set(1,dc_q_base);
    bt_iqimb_dma_enable(st->M0data,fftsize);
    while(1)
    {
        if(iqimb_dma_status==0)
            break;
    }
    caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
    BT_DRV_TRACE(3,"0 dc cal done!!!    dc_i = %d ,dc_q = %d,Energy1 = %d",dc_i_base,dc_q_base,Energy1);

    if(Energy1>200)
    {
        DC_correction(st,&dc_i_base,&dc_q_base,fftsize,dc_i_min,dc_q_min,dc_enery_min);
        bt_Txdc_cal_set(0,dc_i_base);
        bt_Txdc_cal_set(1,dc_q_base);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        BT_DRV_TRACE(3,"1 dc cal done!!!    dc_i = %d ,dc_q = %d,Energy1 = %d",dc_i_base,dc_q_base,Energy1);
    }
    if(Energy1>200)
    {
        DC_correction(st,&dc_i_base,&dc_q_base,fftsize,dc_i_min,dc_q_min,dc_enery_min);
        bt_Txdc_cal_set(0,dc_i_base);
        bt_Txdc_cal_set(1,dc_q_base);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        BT_DRV_TRACE(3,"2 dc cal done!!!    dc_i = %d ,dc_q = %d,Energy1 = %d",dc_i_base,dc_q_base,Energy1);
    }
    g_dc_i_val = dc_i_base;
    g_dc_q_val = dc_q_base;

    bt_update_local_dc_calibration_val(dc_i_base, dc_q_base);
    bt_set_dc_calibration_val_to_nv(&btDcCalibConfig);
#endif
// BT_DRV_TRACE(3,"dc cal done!!!    dc_i = %d ,dc_q = %d,Energy1 = %d",dc_i_base,dc_q_base,Energy1);
    fftsize = 1024;
#ifndef DCCalib
    st = IQMismatchPreprocessState_init(fftsize);
#endif
    Tblgen_iq(st->Table0,st->Table1,st->Table2,fftsize);
    BT_DRV_TRACE(1,"use time: %d ms", __TICKS_TO_MS(hal_sys_timer_get()-time_start));
    for (int k = 0; k<3; k++)
    {
        if(k==0)
        {
            WRITE_REG(0x0,0xD02201E4,0x0);
            WRITE_REG(0x800A000D,0xD02201E4,0x0);
            //osDelay(1000);
            addr = 0xd0310034;
        }
        else if(k==1)
        {
            WRITE_REG(0x0,0xD02201E4,0x0);
            WRITE_REG(0x800A0027,0xD02201E4,0x0);
            addr = 0xd031009c;
        }
        else
        {
            WRITE_REG(0x0,0xD02201E4,0x0);
            WRITE_REG(0x800A0041,0xD02201E4,0x0);
            addr = 0xd0310104;
        }

        btdrv_write_rf_reg(0xAE, 0x0800);
        btdrv_write_rf_reg(0xAE, 0x0000);

        int gain_tmp = 0;
        int phase_tmp = 0 ;
        int gain_min_val = 0;
        int phase_min_val = 0;
        int cur_energy = 1000000;
        //Get gain_min_val

        for(gain_tmp=-128;gain_tmp<=128;gain_tmp=gain_tmp+16)
        {
            bt_iqimb_add_mismatch(0,gain_tmp,phase_tmp,0,0,addr); //no mismatch
            bt_iqimb_dma_enable(st->M0data,fftsize);
            while(1)
            {
                if(iqimb_dma_status==0)
                    break;
            }
            caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
            if(cur_energy>Energy1)
            {
                gain_min_val = gain_tmp;
                cur_energy = Energy1;
            }
#ifdef IQ_CALI_PRINT
            BT_DRV_TRACE(4,"test IQ phase correct addr = 0x%08x,phase_mis = %d ,gain_mis = %d,Energy = %d",addr,phase_tmp,gain_tmp,Energy1);
#endif
        }

        gain_mis_base =  IQ_GAIN_Mismatch_Correction(st,gain_min_val,0,fftsize,addr);
        phase_mis_base = 0;
        bt_iqimb_add_mismatch(0,gain_mis_base,phase_mis_base,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        if(cur_energy>Energy1)
        {
            gain_min_val = gain_mis_base;
            cur_energy = Energy1;
        }

        //Get phase_min_val
        for(phase_tmp=-128;phase_tmp<=128;phase_tmp=phase_tmp+16)
        {
            bt_iqimb_add_mismatch(0,gain_min_val,phase_tmp,0,0,addr); //no mismatch
            bt_iqimb_dma_enable(st->M0data,fftsize);
            while(1)
            {
                if(iqimb_dma_status==0)
                    break;
            }
            caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
            //BT_DRV_TRACE(4,"test IQ phase correct addr = 0x%08x,phase_mis = %d ,gain_mis = %d,Energy = %d",addr,phase_tmp,gain_min_val,Energy1);
            if(cur_energy>Energy1)
            {
                phase_min_val = phase_tmp;
                cur_energy = Energy1;
            }
        }
#ifdef IQ_CALI_PRINT
        BT_DRV_TRACE(2,"gain_val_min = %d,phase_min_val=%d",gain_min_val,phase_min_val);
#endif
        phase_mis_base = IQ_Phase_Mismatch_Correction(st,phase_min_val,gain_min_val,fftsize,addr);

        bt_iqimb_add_mismatch(0,gain_min_val,phase_mis_base,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        if(cur_energy>Energy1)
        {
            phase_min_val = phase_mis_base;
            cur_energy = Energy1;
        }
        else
        {
            Energy_bak = Energy1;
            bt_iqimb_add_mismatch(0,gain_min_val,phase_min_val,0,0,addr); //no mismatch
            bt_iqimb_dma_enable(st->M0data,fftsize);
            while(1)
            {
                if(iqimb_dma_status==0)
                    break;
            }
            caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
            if(Energy1 < Energy_bak)
                phase_mis_base = phase_min_val;
        }
        //BT_DRV_TRACE(3,"0 phase_min = %d ,gain_min = %d,cur_energy = %d",phase_min_val,gain_min_val,cur_energy);
        BT_DRV_TRACE(4,"0 IQ phase correct addr = 0x%08x,phase_mis = %d ,gain_mis = %d,Energy = %d",addr,phase_mis_base,gain_mis_base,Energy1);

        if(Energy1>IQ_ENERGY_THD)
        {
            if(ABS(gain_mis_base)>ABS(phase_mis_base))
            {
                gain_mis_base =  IQ_GAIN_Mismatch_Correction(st,gain_min_val,phase_min_val,fftsize,addr);
                phase_mis_base = IQ_Phase_Mismatch_Correction(st,phase_min_val,gain_mis_base,fftsize,addr);
            }
            else
            {
                phase_mis_base = IQ_Phase_Mismatch_Correction(st,phase_min_val,gain_min_val,fftsize,addr);
                gain_mis_base =  IQ_GAIN_Mismatch_Correction(st,gain_min_val,phase_mis_base,fftsize,addr);
            }

            bt_iqimb_add_mismatch(0,gain_mis_base,phase_mis_base,0,0,addr); //no mismatch
// BT_DRV_TRACE(1,"use time: %d ticks", (hal_sys_timer_get()-time_start));
            bt_iqimb_dma_enable(st->M0data,fftsize);
            while(1)
            {
                if(iqimb_dma_status==0)
                    break;
            }
            caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
            //BT_DRV_TRACE(4,"phase_base = %d ,gain_base = %d,Energy = %d",phase_mis_base,gain_mis_base,Energy1);
            if(cur_energy<Energy1)
            {
                Energy_bak = Energy1;
                bt_iqimb_add_mismatch(0,gain_min_val,phase_min_val,0,0,addr); //no mismatch
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                //BT_DRV_TRACE(4,"phase_min = %d ,gain_min = %d,Energy = %d",phase_min_val,gain_min_val,Energy1);
                if(Energy1 < Energy_bak)
                {
                    phase_mis_base = phase_min_val;
                    gain_mis_base = gain_min_val;
                }
                else
                {
                    phase_min_val = phase_mis_base;
                    gain_min_val = gain_mis_base;
                }

            }
            else
            {
                phase_min_val = phase_mis_base;
                gain_min_val = gain_mis_base;
            }
            BT_DRV_TRACE(4,"1 IQ phase correct addr = 0x%08x,phase_mis = %d ,gain_mis = %d,Energy = %d",addr,phase_mis_base,gain_mis_base,Energy1);
        }

        if(Energy1>1500)
        {
            if(ABS(gain_mis_base)>ABS(phase_mis_base))
            {
                gain_mis_base =  IQ_GAIN_Mismatch_Correction(st,0,0,fftsize,addr);
                phase_mis_base = IQ_Phase_Mismatch_Correction(st,0,gain_mis_base,fftsize,addr);
            }
            else
            {
                phase_mis_base = IQ_Phase_Mismatch_Correction(st,0,0,fftsize,addr);
                gain_mis_base =  IQ_GAIN_Mismatch_Correction(st,0,phase_mis_base,fftsize,addr);
            }
            // BT_DRV_TRACE(1,"use time: %d ticks", (hal_sys_timer_get()-time_start));
            bt_iqimb_dma_enable(st->M0data,fftsize);
            while(1)
            {
                if(iqimb_dma_status==0)
                    break;
            }
            caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
            //BT_DRV_TRACE(4,"phase_base = %d ,gain_base = %d,Energy = %d",phase_mis_base,gain_mis_base,Energy1);
            if(cur_energy<Energy1)
            {
                Energy_bak = Energy1;
                bt_iqimb_add_mismatch(0,gain_min_val,phase_min_val,0,0,addr); //no mismatch
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                //BT_DRV_TRACE(4,"phase_min = %d ,gain_min = %d,Energy = %d",phase_min_val,gain_min_val,Energy1);
                if(Energy1 < Energy_bak)
                {
                    phase_mis_base = phase_min_val;
                    gain_mis_base = gain_min_val;
                }
            }
            else
            {
                phase_min_val = phase_mis_base;
                gain_min_val = gain_mis_base;
            }
            BT_DRV_TRACE(4,"2 IQ phase correct addr = 0x%08x,phase_mis = %d ,gain_mis = %d,Energy = %d",addr,phase_mis_base,gain_mis_base,Energy1);
        }


#ifdef IQ_CALI_DEBUG
        if(2==k)
        {
            while(1)
            {
                hal_sys_timer_delay(MS_TO_TICKS(1000));
                bt_iqimb_add_mismatch(0,0,0,0,0,addr); //no mismatch
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                BT_DRV_TRACE(1,"phase=0,gain=0,Energy = %d",Energy1);
                check_mem_data((uint8_t *)st->M0data,2*1024);
            }
        }
        else
        {
            hal_sys_timer_delay(MS_TO_TICKS(1000));
            bt_iqimb_add_mismatch(0,0,0,0,0,addr); //no mismatch
            bt_iqimb_dma_enable(st->M0data,fftsize);
            while(1)
            {
                if(iqimb_dma_status==0)
                    break;
            }
            caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
            BT_DRV_TRACE(1,"phase=0,gain=0,Energy = %d",Energy1);
            check_mem_data((uint8_t *)st->M0data,2*1024);

        }
#endif


        if(k==0)
        {
            chs= 27;
            addr = addr-13*4;
        }
        else if(k==1)
        {
            chs = 26;
            addr = addr - 12*4;
        }
        else
        {
            chs = 26;
            addr = addr -12*4;
        }

        bt_update_local_iq_calibration_val(k, gain_mis_base, phase_mis_base);

        for (int ch=0; ch<chs; ch++)
        {
            bt_iqimb_add_mismatch(0,gain_mis_base,phase_mis_base,0,0,addr); //no mismatch
            // osDelay(1);
            // BT_DRV_TRACE(4,"IQ phase correct addr = 0x%08x,phase_mis = %d ,gain_mis = %d,Energy = %d",addr,phase_mis_base,gain_mis_base,Energy1);
            addr = addr +4;
        }
        BT_DRV_TRACE(1,"use time: %d ms", __TICKS_TO_MS(hal_sys_timer_get()-time_start));
        //BT_DRV_TRACE(4,"IQ phase correct addr = 0x%08x,phase_mis = %d ,gain_mis = %d,Energy = %d",addr-4,phase_mis_base,gain_mis_base,Energy1);
    }

    IQMismatchPreprocessState_destroy(st);
}

int g_iq_gain,g_iq_phy;

void bt_IQ_DC_Mismatch_Correction_Release_rx()
{
    int fftsize;
    IQState_rx *st = NULL;

    syspool_init();

    syspool_get_buff(&g_medMemPool, MED_MEM_POOL_SIZE);
    med_heap_init(&g_medMemPool[0], MED_MEM_POOL_SIZE);
    //config gpadc 26m
    READ_REG(0xd0310000,0x0);

    fftsize = 4 * 1024;
    st = IQState_init_rx(fftsize);

    bt_iqimb_dma_enable(st->M0data, fftsize / 2);
    while (1) {
        if (iqimb_dma_status == 0)
            break;
    }
    //check_mem_data((uint8_t *)st->M0data, 4 * 1024);

    lib_iqmis_getiq((short *)st->M0data, 4096, (float *)st->data_i, (float *)st->data_q);
    //BT_DRV_TRACE(2, "iq data:0x%x, 0x%x", st->data_i[0], st->data_q[0]);
    //BT_DRV_TRACE(2, "iq data1:0x%x, 0x%x", st->data_i[1], st->data_q[1]);
    lib_iqmis_esti((float *)st->data_i, (float *)st->data_q, 2048, &g_iq_gain, &g_iq_phy);
    //BT_DRV_TRACE(2, "gain:0x%x phy: 0x%x", g_iq_gain, g_iq_phy);

    IQState_init_rx_destroy(st);
}

int bt_iqimb_test_ex (int mismatch_type)
{

    bt_IQ_DC_Mismatch_Correction_Release();
#ifdef IQ_CALI_DEBUG
    while(1);
#endif
    return 1;
}

int bt_iqimb_test_rx (int mismatch_type)
{
    bt_IQ_DC_Mismatch_Correction_Release_rx();
    return 1;
}

#ifdef DCCalib
void bt_dccalib_set_value(void)
{
    BT_DRV_TRACE(3, "%s Txdc_cal_set: i=%d, q=%d", __func__, g_dc_i_val, g_dc_q_val);
    bt_Txdc_cal_set(0, g_dc_i_val);
    bt_Txdc_cal_set(1, g_dc_q_val);
}
#endif

#endif//TX_IQ_CAL
#endif

