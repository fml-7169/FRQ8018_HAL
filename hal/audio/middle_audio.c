/*
 * middle_audio.c
 *
 *  Created on: 2020-9-27
 *      Author: Weili.Hu
 */

#include <string.h>
#include "types.h"
#include "govee_log.h"
#include "middle_audio.h"
#include "Lite-Rbuffer.h"

#define AUDIO_DATA_BUFFER_SIZE      2048

static LR_handler gt_lr_handler = NULL;

static void audio_sample_read_adc(int16* buffer, uint32 size)
{
    int32 ret = 0;

    ret = Lite_ring_buffer_write_data(gt_lr_handler, (uint8*)buffer, size*GOVEE_AUDIO_SAMPLE_SIZE);
    if (ret != 0)
    {
        //GOVEE_PRINT(LOG_DEBUG, "Write audio data failed.\r\n");
    }
}

int32 mid_audio_sample_read(int16* buffer, uint32 sample_size)
{
    int32 data_len = 0;

    data_len = Lite_ring_buffer_size_get(gt_lr_handler);
    if (data_len < sample_size*GOVEE_AUDIO_SAMPLE_SIZE)
    {
        //GOVEE_PRINT(LOG_DEBUG, "audio data is not enough.\r\n");
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

int32 mid_audio_init(audio_config_t* pt_audio)
{
    gt_lr_handler = Lite_ring_buffer_init(AUDIO_DATA_BUFFER_SIZE);
    if (gt_lr_handler == NULL)
    {
        GOVEE_PRINT(LOG_ERROR, "Audio ring buffer init failed.\r\n");
        return -1;
    }

    pt_audio->audio_read = audio_sample_read_adc;

    return 0;
}
