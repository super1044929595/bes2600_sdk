#include "hal_codec.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_aud.h"
#include "hal_timer.h"
#include "hal_sleep.h"
#include "analog.h"
#include "audioflinger.h"
//#include "app_audio.h"
#include "app_utils.h"
#include "voice_detector.h"
#include <string.h>
#include "audio_dump.h"
#include "speech_ssat.h"

#include "pmu.h"
#include "CSpotterSDKApi.h"

#ifdef I2C_VAD
#include "vad_sensor.h"
#endif

/* This macro is used to show trace info for debugging */
#define VD_DEBUG

#ifdef VD_DEBUG
#define VD_LOG TRACE
#else
#define VD_LOG(...) do{}while(0)
#endif

#ifdef VD_TEST
#define VD_TRACE TRACE
#else
#define VD_TRACE(...) do{}while(0)
#endif

#define VAD_RAW_DATA_BUFF_SIZE (1024 * 8)

#define CMD_QUEUE_DATA_SIZE 15

struct command_queue {
    int idx;
    int out;
    int data[CMD_QUEUE_DATA_SIZE];
};

struct voice_detector_dev {
    uint8_t init;
    uint8_t dfl;
    uint8_t run;
    enum voice_detector_state state;
    uint32_t wakeup_cnt;
    uint32_t arguments[VOICE_DET_CB_QTY];
    voice_detector_cb_t callback[VOICE_DET_CB_QTY];
    struct command_queue cmd_queue;
    struct codec_vad_conf conf;
    struct AF_STREAM_CONFIG_T cap_conf;
    struct AF_STREAM_CONFIG_T ply_conf;
    uint32_t vad_raw_data_buff[VAD_RAW_DATA_BUFF_SIZE / 4];
    uint32_t vad_raw_data_len;
    uint32_t sys_clk;
    uint32_t aon_clk_en;
};

static struct voice_detector_dev voice_det_devs[VOICE_DETECTOR_QTY];

short vad_buf[2048] = {0};
unsigned int vad_buf_len = 0;

static uint32_t POSSIBLY_UNUSED cyb_buf[20*1024];
static uint32_t POSSIBLY_UNUSED cyb_buf_used = 0;

extern unsigned char __g_cybase_start[];
extern unsigned char __g_cygroup_start[];
#define BASE_MODEL_DATA (__g_cybase_start)
#define COMMAND_MODEL_DATA  (__g_cygroup_start)
HANDLE h_CSpotter = NULL;


static void voice_detector_vad_callback(uint8_t *pbuf, uint32_t len, uint32_t irq_status);
static void voice_detector_exec_callback(struct voice_detector_dev *pdev,
                enum voice_detector_cb_id id);

#define to_voice_dev(id) (&voice_det_devs[(id)])

static int cmd_queue_enqueue(struct command_queue *q, int c)
{
    if (q->idx >= CMD_QUEUE_DATA_SIZE) {
        VD_LOG("%s, overflow cmd=%d", __func__, c);
        return -2;
    }
    if (q->idx < 0) {
        q->idx = 0;
    }
    q->data[q->idx++] = c;
//    VD_LOG("%s, cmd=%d", __func__, c);
    return 0;
}

static int cmd_queue_dequeue(struct command_queue *q)
{
    int cmd;

    if (q->idx < 0) {
//        VD_LOG("%s, empty", __func__);
        return -2;
    }
    if (q->out < 0) {
        q->out = 0;
    }
    cmd = q->data[q->out++];
    if (q->out >= q->idx) {
        q->out = -1;
        q->idx = -1;
    }
//    VD_LOG("%s, cmd=%d", __func__, cmd);
    return cmd;
}

static int cmd_queue_is_empty(struct command_queue *q)
{
    return (q->idx < 0) ? 1 : 0;
}

static void voice_detector_init_cmd_queue(struct command_queue *q)
{
    if (q) {
        q->idx = -1;
        q->out = -1;
    }
}

static void voice_detector_init_vad(struct codec_vad_conf *c, int id)
{
    if (c) {
        c->udc         = VAD_DEFAULT_UDC;
        c->upre        = VAD_DEFAULT_UPRE;
#ifdef I2C_VAD
        c->frame_len   = VAD_DEFAULT_SENSOR_FRAME_LEN;
#else
        c->frame_len   = VAD_DEFAULT_MIC_FRAME_LEN;
#endif
        c->mvad        = VAD_DEFAULT_MVAD;
        c->dig_mode    = VAD_DEFAULT_DIG_MODE;
        c->pre_gain    = VAD_DEFAULT_PRE_GAIN;
        c->sth         = VAD_DEFAULT_STH;
        c->dc_bypass   = VAD_DEFAULT_DC_BYPASS;
        c->frame_th[0] = VAD_DEFAULT_FRAME_TH0;
        c->frame_th[1] = VAD_DEFAULT_FRAME_TH0;
        c->frame_th[2] = VAD_DEFAULT_FRAME_TH0;
        c->range[0]    = VAD_DEFAULT_RANGE0;
        c->range[1]    = VAD_DEFAULT_RANGE1;
        c->range[2]    = VAD_DEFAULT_RANGE2;
        c->range[3]    = VAD_DEFAULT_RANGE3;
        c->psd_th[0]   = VAD_DEFAULT_PSD_TH0;
        c->psd_th[1]   = VAD_DEFAULT_PSD_TH1;
        c->handler     = voice_detector_vad_callback;
        c->buf_addr    = NULL;
        c->buf_size    = 0;
    }
}

int voice_detector_open(enum voice_detector_id id)
{
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        VD_LOG("%s, invalid id=%d", __func__, id);
        return -1;
    }

    pdev = to_voice_dev(id);
    if (pdev->init) {
        VD_LOG("%s, dev already open", __func__);
        return -2;
    }
    if (pdev->run) {
        VD_LOG("%s, dev not stoped", __func__);
        return -3;
    }

    memset(pdev, 0x0, sizeof(struct voice_detector_dev));
    memset(&pdev->cap_conf, 0x0, sizeof(struct AF_STREAM_CONFIG_T));
    memset(&pdev->ply_conf, 0x0, sizeof(struct AF_STREAM_CONFIG_T));
    voice_detector_init_cmd_queue(&pdev->cmd_queue);
    voice_detector_init_vad(&pdev->conf, id);
    pdev->state = VOICE_DET_STATE_IDLE;
    pdev->run  = 0;
    pdev->dfl  = 1;
    pdev->init = 1;

    return 0;
}

void voice_detector_close(enum voice_detector_id id)
{
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        VD_LOG("%s, invalid id=%d", __func__, id);
        return;
    }

    pdev = to_voice_dev(id);
    if (pdev->init) {
        if (pdev->run) {
            cmd_queue_enqueue(&pdev->cmd_queue, VOICE_DET_CMD_EXIT);
        }
        pdev->init = 0;
    }
}

int voice_detector_setup_vad(enum voice_detector_id id,
        struct codec_vad_conf *conf)
{
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        VD_LOG("%s, invalid id=%d", __func__, id);
        return -1;
    }

    pdev = to_voice_dev(id);
    if (!pdev->init) {
        VD_LOG("%s, dev not open", __func__);
        return -2;
    }

    if (conf) {
        memcpy(&pdev->conf, conf, sizeof(struct codec_vad_conf));
        pdev->conf.handler = voice_detector_vad_callback;
        pdev->conf.buf_addr = NULL;
        pdev->conf.buf_size = 0;
        pdev->dfl = 0;
    }
    return 0;
}

int voice_detector_setup_stream(enum voice_detector_id id,
        enum AUD_STREAM_T stream_id, struct AF_STREAM_CONFIG_T *stream)
{
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        VD_LOG("%s, invalid id=%d", __func__, id);
        return -1;
    }

    pdev = to_voice_dev(id);
    if (!pdev->init) {
        VD_LOG("%s, dev not open", __func__);
        return -2;
    }

    if (stream_id == AUD_STREAM_CAPTURE)
        memcpy(&pdev->cap_conf, stream, sizeof(struct AF_STREAM_CONFIG_T));
    else
        memcpy(&pdev->ply_conf, stream, sizeof(struct AF_STREAM_CONFIG_T));

    return 0;
}

int voice_detector_setup_callback(enum voice_detector_id id,
        enum voice_detector_cb_id func_id, voice_detector_cb_t func, void *param)
{
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        VD_LOG("%s, invalid id=%d", __func__, id);
        return -1;
    }

    pdev = to_voice_dev(id);
    if (!pdev->init) {
        VD_LOG("%s, dev not open", __func__);
        return -2;
    }

    if (func_id >= VOICE_DET_CB_QTY) {
        VD_LOG("%s, invalid func_id=%d", __func__, func_id);
        return -3;
    }
    pdev->arguments[func_id] = (uint32_t)param;
    pdev->callback[func_id]  = func;
    return 0;
}

int voice_detector_send_cmd(enum voice_detector_id id, enum voice_detector_cmd cmd)
{
    int r;
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        VD_LOG("%s, invalid id=%d", __func__, id);
        return -1;
    }
    pdev = to_voice_dev(id);

    r = cmd_queue_enqueue(&pdev->cmd_queue, (int)cmd);
    return r;
}

int voice_detector_send_cmd_array(enum voice_detector_id id, int *cmd_array, int num)
{
    int r, i;
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        VD_LOG("%s, invalid id=%d", __func__, id);
        return -1;
    }
    pdev = to_voice_dev(id);

    for(i = 0; i < num; i++) {
        r = cmd_queue_enqueue(&pdev->cmd_queue, cmd_array[i]);
        if (r)
            return r;
    }
    return 0;
}

enum voice_detector_state voice_detector_query_status(enum voice_detector_id id)
{
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        return VOICE_DET_STATE_IDLE;
    }
    pdev = to_voice_dev(id);

    VD_LOG("%s, state=%d", __func__, pdev->state);
    return pdev->state;
}

int voice_detector_recv_vad_data(enum voice_detector_id id,
                            uint8_t *pbuf, uint32_t buf_size)
{
    struct voice_detector_dev *pdev;
    uint32_t len = 0;

    if (id >= VOICE_DETECTOR_QTY) {
        VD_LOG("%s, invalid id=%d", __func__, id);
        return 0;
    }
    pdev = to_voice_dev(id);

    if (pbuf) {
        len = pdev->vad_raw_data_len;
        if (!len)
            return 0;

        if (len > buf_size)
            len = buf_size;

        memcpy(pbuf, pdev->vad_raw_data_buff, len);
        pdev->vad_raw_data_len = 0;
    }
    VD_LOG("%s, len=%d", __func__, len);
    return len;
}

static void voice_detector_vad_callback(uint8_t *pbuf, uint32_t len, uint32_t irq_status)
{
    struct voice_detector_dev *pdev;
    enum voice_detector_id id = VOICE_DETECTOR_ID_0;

    pdev = to_voice_dev(id);

    pdev->wakeup_cnt++;

    VD_LOG("%s, voice detector[%d], wakeup_cnt=%d",
            __func__, id, pdev->wakeup_cnt);

    if (pdev->wakeup_cnt == 1) {
        /*
         * VOICE_DET_CB_APP should be called only once
         * after CPU is waked up from sleeping.
         * The VAD already can gernerates interrupts at this monment or later.
         */
        if((irq_status & VAD_FIND))
            voice_detector_exec_callback(pdev, VOICE_DET_FIND_APP);
         else if((irq_status & VAD_NOT_FIND))
            voice_detector_exec_callback(pdev, VOICE_DET_NOT_FIND_APP);
    }
}

static int voice_detector_vad_open(struct voice_detector_dev *pdev)
{
    struct codec_vad_conf *c = &pdev->conf;

    if (pdev->dfl) {
        VD_LOG("%s, use dfl vad", __func__);
    }
    hal_codec_vad_open();
    hal_codec_vad_config(c);
#ifdef I2C_VAD
    vad_sensor_open();
#endif
    return 0;
}

static int voice_detector_vad_start(struct voice_detector_dev *pdev)
{
    /* wakeup_cnt is cleared while VAD starts */
    pdev->wakeup_cnt = 0;
    hal_codec_vad_start();
#ifdef I2C_VAD
    vad_sensor_engine_start();
#endif
    return 0;
}

static int voice_detector_vad_stop(struct voice_detector_dev *pdev)
{
    uint8_t *pbuf = (uint8_t *)pdev->vad_raw_data_buff;
    //uint32_t* test_buf = (uint32_t *)pdev->vad_raw_data_buff;
    uint32_t len;

#ifdef I2C_VAD
    vad_sensor_engine_stop();
    len = 0;
#else
    /* receive vad data before stop it */
    len = hal_codec_vad_recv_data(pbuf, VAD_RAW_DATA_BUFF_SIZE);
#endif

    pdev->vad_raw_data_len = len;

    //for(int i = 0; i < 2048; i = i+320)
    //    TRACE(2,"0x%x, 0x%x", i, test_buf[i]);

    for(unsigned int i = 0; i < len/4; i++)
        vad_buf[i] = (short)(((int*)pbuf)[i]>>12);
    vad_buf_len = len/2;
    hal_codec_vad_stop();
    return 0;
}

static int voice_detector_vad_close(struct voice_detector_dev *pdev)
{
#ifdef I2C_VAD
    vad_sensor_close();
#endif
    hal_codec_vad_close(0);
    return 0;
}

static int voice_detector_aud_cap_open(struct voice_detector_dev *pdev)
{
    struct AF_STREAM_CONFIG_T *conf = &pdev->cap_conf;

    if ((!conf->handler) || (!conf->data_ptr)) {
        VD_LOG("%s, capture stream is null", __func__);
        return -1;
    }
    af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, conf);
    return 0;
}

static int voice_detector_aud_cap_start(struct voice_detector_dev *pdev)
{
    af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    return 0;
}

static int voice_detector_aud_cap_stop(struct voice_detector_dev *pdev)
{
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    return 0;
}

static int voice_detector_aud_cap_close(struct voice_detector_dev *pdev)
{
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    return 0;
}

static int voice_detector_sys_clk(struct voice_detector_dev *pdev)
{
#ifdef VD_TEST
    app_sysfreq_req(APP_SYSFREQ_USER_APP_0, (enum APP_SYSFREQ_FREQ_T)(pdev->sys_clk));
#else
    app_sysfreq_req(APP_SYSFREQ_USER_APP_KWS, (enum APP_SYSFREQ_FREQ_T)(pdev->sys_clk));
#endif
//    VD_TRACE(2,"%s, cpu freq=%d", __func__, hal_sys_timer_calc_cpu_freq(5,0));
    return 0;
}

static int voice_detector_aud_aon_clk(struct voice_detector_dev *pdev)
{
    hal_cmu_codec_hclk_to_aon(pdev->aon_clk_en);
    return 0;
}

static int voice_detector_exit(struct voice_detector_dev *pdev)
{
    //TODO: exit process
    return 0;
}

static int voice_detector_idle(struct voice_detector_dev *pdev)
{
    //TODO: idle process
    return 0;
}

struct cmd_vector {
    const char *name;
    int cmd;
    int (*func)(struct voice_detector_dev *pdev);
};

static struct cmd_vector cmd_vectors[] = {
    {"idle",     VOICE_DET_CMD_IDLE,           voice_detector_idle},
    {"exit",     VOICE_DET_CMD_EXIT,           voice_detector_exit},
    {"vad open", VOICE_DET_CMD_VAD_OPEN,       voice_detector_vad_open},
    {"vad start",VOICE_DET_CMD_VAD_START,      voice_detector_vad_start},
    {"vad stop", VOICE_DET_CMD_VAD_STOP,       voice_detector_vad_stop},
    {"vad close",VOICE_DET_CMD_VAD_CLOSE,      voice_detector_vad_close},
    {"cap start",VOICE_DET_CMD_AUD_CAP_START,  voice_detector_aud_cap_start},
    {"cap stop", VOICE_DET_CMD_AUD_CAP_STOP,   voice_detector_aud_cap_stop},
    {"cap open", VOICE_DET_CMD_AUD_CAP_OPEN,   voice_detector_aud_cap_open},
    {"cap close",VOICE_DET_CMD_AUD_CAP_CLOSE,  voice_detector_aud_cap_close},
    {"hclk on",  VOICE_DET_CMD_AUD_AON_CLK_EN, voice_detector_aud_aon_clk},
    {"hclk off", VOICE_DET_CMD_AUD_AON_CLK_DIS,voice_detector_aud_aon_clk},
    {"clk32k",   VOICE_DET_CMD_SYS_CLK_32K,    voice_detector_sys_clk},
    {"clk26m",   VOICE_DET_CMD_SYS_CLK_26M,    voice_detector_sys_clk},
    {"clk52m",   VOICE_DET_CMD_SYS_CLK_52M,    voice_detector_sys_clk},
    {"clk104m",  VOICE_DET_CMD_SYS_CLK_104M,   voice_detector_sys_clk},
};

static int voice_detector_process_cmd(struct voice_detector_dev *pdev, int cmd)
{
    int err;

    VD_LOG("%s, cmd[%d]: %s", __func__, cmd, cmd_vectors[(int)cmd].name);

    switch(cmd) {
    case VOICE_DET_CMD_AUD_AON_CLK_EN:
        pdev->aon_clk_en = 1;
        break;
    case VOICE_DET_CMD_AUD_AON_CLK_DIS:
        pdev->aon_clk_en = 0;
        break;
    case VOICE_DET_CMD_SYS_CLK_32K:
        pdev->sys_clk = APP_SYSFREQ_32K;
        break;
    case VOICE_DET_CMD_SYS_CLK_26M:
        pdev->sys_clk = APP_SYSFREQ_26M;
        break;
    case VOICE_DET_CMD_SYS_CLK_52M:
        pdev->sys_clk = APP_SYSFREQ_52M;
        break;
    case VOICE_DET_CMD_SYS_CLK_104M:
        pdev->sys_clk = APP_SYSFREQ_104M;
        break;
    default:
        break;
    }
    err = cmd_vectors[(int)cmd].func(pdev);
    return err;
}

static void voice_detector_set_status(struct voice_detector_dev *pdev, enum voice_detector_state s)
{
    if ((s == VOICE_DET_STATE_SYS_CLK_104M)
            || (s == VOICE_DET_STATE_SYS_CLK_52M)
            || (s == VOICE_DET_STATE_SYS_CLK_26M)
            || (s == VOICE_DET_STATE_SYS_CLK_32K)) {
        return;
    }
    pdev->state = s;
    VD_LOG("%s, state=%d", __func__, pdev->state);
}

static void voice_detector_exec_callback(struct voice_detector_dev *pdev,
                enum voice_detector_cb_id id)
{
    voice_detector_cb_t func;

    func = pdev->callback[id];
    if (func) {
        void *argv = (void *)(pdev->arguments[id]);

        func(pdev->state, argv);
    }
}

int voice_detector_enhance_perform(enum voice_detector_id id)
{
    struct voice_detector_dev *pdev = to_voice_dev(id);

    pdev->sys_clk = APP_SYSFREQ_26M;

    return app_sysfreq_req(APP_SYSFREQ_USER_APP_KWS,
                (enum APP_SYSFREQ_FREQ_T)(pdev->sys_clk));
}

int voice_detector_run(enum voice_detector_id id, int continous)
{
    int exit = 0;
    int exit_code = 0;
    struct voice_detector_dev *pdev;

    if (id >= VOICE_DETECTOR_QTY) {
        return -1;
    }

    pdev = to_voice_dev(id);
    if (!pdev->init)
        return -2;

    voice_detector_exec_callback(pdev, VOICE_DET_CB_PREVIOUS);
    pdev->run = 1;
    while (1) {
        int err = 0;
        int cmd = cmd_queue_dequeue(&pdev->cmd_queue);

        if (cmd < 0) {
            // cmd is invalid, invoke waitting callback
            voice_detector_exec_callback(pdev, VOICE_DET_CB_RUN_WAIT);
        } else {
            // cmd is okay, process it
            err = voice_detector_process_cmd(pdev, cmd);
            if (!err) {
                voice_detector_set_status(pdev, (enum voice_detector_state)cmd);
                voice_detector_exec_callback(pdev, VOICE_DET_CB_RUN_DONE);
            } else {
                voice_detector_exec_callback(pdev, VOICE_DET_CB_ERROR);
                VD_LOG("%s, process cmd %d error, %d", __func__, cmd, err);
            }
        }
        switch(continous) {
        case VOICE_DET_MODE_ONESHOT:
            // not continous, run only once
            exit_code = err;
            exit = 1;
            break;
        case VOICE_DET_MODE_EXEC_CMD:
            // continous run,exit until cmd queue is empty
            if (cmd_queue_is_empty(&pdev->cmd_queue)) {
                exit = 1;
                exit_code = err;
                // invoke waitting callback
                voice_detector_exec_callback(pdev, VOICE_DET_CB_RUN_WAIT);
            }
            break;
        case VOICE_DET_MODE_LOOP:
            //continous run forever, exit until receive VOICE_DET_CMD_EXIT
            if (cmd == VOICE_DET_CMD_EXIT) {
                exit = 1;
                exit_code = err;
            }
            break;
        default:
            break;
        }
        if (exit) {
            break;
        }
    }
    pdev->run = 0;
    voice_detector_exec_callback(pdev, VOICE_DET_CB_POST);

    return exit_code;
}

#ifdef VD_TEST

#include "app_voice_detector.h"

#define AUDIO_CAP_BUFF_SIZE (160*2*2*2)

static uint32_t buff_capture[AUDIO_CAP_BUFF_SIZE/4];
static uint32_t voice_det_evt = 0;
static uint8_t  vad_data_buf[8 * 1024];

static void voice_detector_send_evt(uint32_t evt)
{
    voice_det_evt = evt;
}

static void print_vad_raw_data(uint8_t *buf, uint32_t len)
{
    VD_TRACE(3,"%s, buf=%x, len=%d", __func__, (uint32_t)buf, len);
    //TODO: print data
}


static int State_M_1 = 0;
void dc_filter_f(short *in, int len, float left_gain, float right_gain)
{
	int tmp1;
	for (int i = 0; i<len; i+=1)
	{
		State_M_1 = (15 * State_M_1 + in[i]) >> 4;
		tmp1 = in[i];
		tmp1 -= State_M_1;
		in[i] = speech_ssat_int16(tmp1);
	}
}


static uint32_t mic_data_come(uint8_t *buf, uint32_t len)
{
    static int come_cnt = 0;
    short *p16data = (short *)buf;
    uint32_t sample_len = len / 2;
    int retcode;

    dc_filter_f(p16data, sample_len, 0.0, 0.0);

    audio_dump_clear_up();
    audio_dump_add_channel_data(0, p16data, sample_len);
    //audio_dump_add_channel_data(1, temp_buf, sample_len);	
    audio_dump_run();




    retcode = CSpotter_AddSample(h_CSpotter, p16data, sample_len);

    if (retcode == CSPOTTER_SUCCESS) {
        int id, score;

        id = CSpotter_GetResult(h_CSpotter);
        TRACE(1,"##### CSpotterGetResult return ID : %d\n", id);

        score = CSpotter_GetResultScore(h_CSpotter);
        TRACE(1,"CSpotter_GetResultScore return Score: %d", score);

        CSpotter_Reset(h_CSpotter);

    }

    if(come_cnt % 100 == 0) {
        TRACE(1,"retcode: %d", retcode);
    }
    come_cnt++;
    //if ((come_cnt % 200) == 0) {
    //    VD_TRACE(3,"%s, buf=%x, len=%d", __func__, (uint32_t)buf, len);
   // }
    if ((come_cnt % 300) == 0) {
        come_cnt = 0;
        voice_detector_send_evt(VOICE_DET_EVT_VAD_START);
        VD_TRACE(1,"%s, close audio stream ...", __func__);
    }

    return 0;
}


static int CSpotter_Init_bes()
{
#define TIMES (1000)
    int err;
    int state_size, mem_size;
    uint8_t *state_buffer, *mem_pool;
    uint8_t *p_combuf = (uint8_t *)cyb_buf;

    TRACE(1,"%s",__func__);

    state_size = CSpotter_GetStateSize((BYTE*)COMMAND_MODEL_DATA);
    TRACE(2,"%s, state_size=%d",__func__, state_size);

    state_buffer = p_combuf+ cyb_buf_used;
    cyb_buf_used += state_size;   
    if (!state_buffer) {
        TRACE(0,"alloc state buff failed");
        err = -1;
        goto fail;
    }

    mem_size = CSpotter_GetMemoryUsage_Sep((BYTE*)BASE_MODEL_DATA, (BYTE*)COMMAND_MODEL_DATA, TIMES);
    TRACE(1,"mem_size=%d",mem_size);

    mem_pool = p_combuf+ cyb_buf_used;
    cyb_buf_used += mem_size;
    if (!mem_pool) {
        TRACE(0,"alloc mem pool failed");
        err = -2;
        goto fail;
    }

    h_CSpotter = CSpotter_Init_Sep((BYTE*)BASE_MODEL_DATA, (BYTE*)COMMAND_MODEL_DATA,
            TIMES, mem_pool, mem_size, state_buffer, state_size, &err);
    if (!h_CSpotter) {
        TRACE(1,"CSpotter Init fail! err : %d\n", err);
        err = -3;
        goto fail;
    }

    return 0;

fail:
    return err;
}


static void cmd_wait_handler(int state, void *param)
{
    voice_detector_send_evt(VOICE_DET_EVT_IDLE);

    VD_TRACE(2,"%s, state=%d", __func__, state);
//    hal_sys_timer_delay(MS_TO_TICKS(100));
//    while(1);

//    hal_sleep_enter_sleep();
}

static void cmd_done_handler(int state, void *param)
{
    VD_TRACE(2,"%s, state=%d", __func__, state);
}

static void cpu_det_find_wakeup_handler(int state, void *param)
{
    static uint32_t cpu_wakeup_cnt = 0;

    cpu_wakeup_cnt++;
    VD_TRACE(3,"%s, state=%d, cnt=%d", __func__, state, cpu_wakeup_cnt);
    //VD_TRACE(2,"%s, calc sys freq=%d", __func__, hal_sys_timer_calc_cpu_freq(5,0));

    voice_detector_send_evt(VOICE_DET_EVT_AUD_CAP_START);
}

static void cpu_det_notfind_wakeup_handler(int state, void *param)
{

    VD_TRACE(2,"%s, state=%d", __func__, state);
    //VD_TRACE(2,"%s, calc sys freq=%d", __func__, hal_sys_timer_calc_cpu_freq(5,0));

    voice_detector_send_evt(VOICE_DET_EVT_VAD_START);
}

void voice_detector_test(void)
{
    enum voice_detector_id id = VOICE_DETECTOR_ID_0;
    int r, run;
    struct AF_STREAM_CONFIG_T stream_cfg;
    enum HAL_SLEEP_STATUS_T sleep;
    uint32_t len;

    VD_TRACE(1,"%s, start", __func__);

    analog_open();
    af_open();

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.sample_rate = AUD_SAMPRATE_16000;
    stream_cfg.bits        = AUD_BITS_16;
    stream_cfg.vol = 16;
    stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.io_path     = AUD_INPUT_PATH_VADMIC;
    stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
    stream_cfg.handler     = mic_data_come;
    stream_cfg.data_ptr    = (uint8_t *)buff_capture;
    stream_cfg.data_size   = AUDIO_CAP_BUFF_SIZE;

    audio_dump_init(AUDIO_CAP_BUFF_SIZE / 2 / 2, sizeof(short), 1);

    CSpotter_Init_bes();
    r = voice_detector_open(id);
    if (r) {
        VD_TRACE(2,"%s, error %d", __func__, r);
        return;
    }

    voice_detector_setup_stream(id, AUD_STREAM_CAPTURE, &stream_cfg);
    voice_detector_setup_callback(id, VOICE_DET_CB_RUN_WAIT, cmd_wait_handler, NULL);
    voice_detector_setup_callback(id, VOICE_DET_CB_RUN_DONE, cmd_done_handler, NULL);
    voice_detector_setup_callback(id, VOICE_DET_FIND_APP, cpu_det_find_wakeup_handler, NULL);
    voice_detector_setup_callback(id, VOICE_DET_NOT_FIND_APP, cpu_det_notfind_wakeup_handler, NULL);

    VD_TRACE(2,"%s, calc sys freq=%d", __func__, hal_sys_timer_calc_cpu_freq(5,0));

    voice_detector_send_cmd(id, VOICE_DET_CMD_AUD_CAP_OPEN);
    voice_detector_send_cmd(id, VOICE_DET_CMD_AUD_CAP_CLOSE);
    voice_detector_send_evt(VOICE_DET_EVT_VAD_START);
    run = 0;

    while (1) {
        if (voice_det_evt != VOICE_DET_EVT_IDLE) {
            switch(voice_det_evt) {
            case VOICE_DET_EVT_VAD_START:
                if (voice_detector_query_status(id) == VOICE_DET_STATE_VAD_CLOSE) {
                    voice_detector_send_cmd(id, VOICE_DET_CMD_AUD_CAP_STOP);
                    voice_detector_send_cmd(id, VOICE_DET_CMD_AUD_CAP_CLOSE);
                }
                //TODO: save current system clock
                voice_detector_send_cmd(id, VOICE_DET_CMD_VAD_OPEN);
                voice_detector_send_cmd(id, VOICE_DET_CMD_VAD_START);
                voice_detector_send_cmd(id, VOICE_DET_CMD_SYS_CLK_32K);
                run = 1;
                break;
            case VOICE_DET_EVT_AUD_CAP_START:
                voice_detector_send_cmd(id, VOICE_DET_CMD_SYS_CLK_26M);
                voice_detector_send_cmd(id, VOICE_DET_CMD_AUD_CAP_OPEN);
                voice_detector_send_cmd(id, VOICE_DET_CMD_AUD_CAP_START);
                voice_detector_send_cmd(id, VOICE_DET_CMD_VAD_STOP);
                voice_detector_send_cmd(id, VOICE_DET_CMD_VAD_CLOSE);
                //voice_detector_send_cmd(id, VOICE_DET_CMD_AUD_CAP_OPEN);
                //voice_detector_send_cmd(id, VOICE_DET_CMD_AUD_CAP_START);
                run = 1;
                break;
            default:
                run = 0;
                break;
            }
            voice_det_evt = VOICE_DET_EVT_IDLE;
        }
        if (run) {
            run = 0;
            voice_detector_run(id, VOICE_DET_MODE_EXEC_CMD);
        }

//      while(1);
        sleep = hal_sleep_enter_sleep();
        if (sleep == HAL_SLEEP_STATUS_DEEP) {
            VD_TRACE(0,"wake up from deep sleep");
        }

        len = voice_detector_recv_vad_data(VOICE_DETECTOR_ID_0,
                vad_data_buf, sizeof(vad_data_buf));
        if (len) {
            print_vad_raw_data(vad_data_buf, len);
        }
    }
}
#endif
