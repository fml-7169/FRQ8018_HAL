/*
 * middle_audio.c
 *
 *  Created on: 2020-9-27
 *      Author: Weili.Hu
 */

#include <stdint.h>
#include "os_task.h"
#include "os_msg_q.h"
#include "co_printf.h"
#include "driver_plf.h"
#include "driver_system.h"
#include "driver_i2s.h"
#include "driver_pmu.h"
#include "driver_codec.h"
#include "driver_uart.h"
#include "driver_ssp.h"
#include "driver_frspim.h"
#include "driver_codec.h"
#include <string.h>

#include "govee_log.h"
#include "middle_audio.h"
#include "Lite-Rbuffer.h"

#define I2S_IRQ_PRIO    4
#define AUDIO_DATA_BUFFER_SIZE      1024

#define codec_write(addr, data)         frspim_wr(FR_SPI_CODEC_CHAN, addr, 1, (uint32_t)data)

static LR_handler gt_lr_handler = NULL;
static int16_t audio_data[I2S_FIFO_DEPTH/2];

static int gGain;
static int gMaxGain_sensitive = 0x2F;

extern void codec_adc_init(uint8_t sample_rate);

__attribute__((section("ram_code"))) void i2s_isr_ram(void)
{
    static int total_value = 0;
    static uint16_t sample_count = 0;
    if(i2s_get_int_status() & I2S_INT_STATUS_RX_HFULL) {//codec_ADC
        i2s_get_data(audio_data, I2S_FIFO_DEPTH/2, I2S_DATA_MONO);
        Lite_ring_buffer_write_data(gt_lr_handler, (uint8*)audio_data, I2S_FIFO_DEPTH);
    }
}


int32 mid_audio_sample_read(int16* buffer, uint32 sample_size)
{
    int32 data_len = 0;

    data_len = Lite_ring_buffer_size_get(gt_lr_handler);
    if (data_len < sample_size*GOVEE_AUDIO_SAMPLE_SIZE)
    {
        GOVEE_PRINT(LOG_DEBUG, "audio data is not enough.\r\n");
        return -1;
    }
    Lite_ring_buffer_read_data(gt_lr_handler, (uint8*)buffer, sample_size*GOVEE_AUDIO_SAMPLE_SIZE);

    return 0;
}

int32 mid_audio_sample_size(void)
{
    int32 data_size = Lite_ring_buffer_size_get(gt_lr_handler);

    return data_size / GOVEE_AUDIO_SAMPLE_SIZE;
}

int32 mid_audio_init(void)
{
    pmu_set_aldo_voltage(PMU_ALDO_MODE_BYPASS, 0x00);
    pmu_codec_power_enable();
    codec_adc_init(CODEC_SAMPLE_RATE_8000);
    codec_write(0x18, 0x1C);//PGA  P/N exchange, P enable  N enable
    codec_enable_adc();
    //codec_enable_dac();
    i2s_init(I2S_DIR_RX,8000,1);
    i2s_start();
    NVIC_SetPriority(I2S_IRQn,I2S_IRQ_PRIO);
    NVIC_EnableIRQ(I2S_IRQn);

    audio_sensitivity(100);
    gt_lr_handler = Lite_ring_buffer_init(AUDIO_DATA_BUFFER_SIZE);
    if (gt_lr_handler == NULL)
    {
        GOVEE_PRINT(LOG_ERROR, "Audio ring buffer init failed.\r\n");
        return -1;
    }
    return 0;
}

void audio_sensitivity(int level){
    if(level > 100)level = 100;
    gMaxGain_sensitive = level*0x3F/100;
    gGain = gMaxGain_sensitive;
    codec_write(0x19, (unsigned char)gGain);
}

