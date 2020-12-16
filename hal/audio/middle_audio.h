#ifndef MIDDLE_AUDIO_H_
#define MIDDLE_AUDIO_H_
#include "hal_types.h"


#define GOVEE_AUDIO_SAMPLE_SIZE 2

typedef void (*audio_callback)(int16* p_data, uint32 sample_size);

typedef struct
{
    audio_callback audio_read;
} audio_config_t;

int32 mid_audio_sample_read(int16* buffer, uint32 sample_size);
int32 mid_audio_sample_size(void);
int32 mid_audio_init(void);
void audio_sensitivity(int level);

#endif /* MIDDLE_AUDIO_H_ */