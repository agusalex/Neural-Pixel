// constants.c
#include <stdio.h>
#include "constants.h"

const char* SD_FILES_PATH = "./models/";
const char* MODELS_PATH = "./models/checkpoints/";
const char* CLIPS_PATH = "./models/clips/";
const char* CONTROLNET_PATH = "./models/controlnet/";
const char* EMBEDDINGS_PATH = "./models/embeddings/";
const char* LORAS_PATH = "./models/loras/";
const char* TEXT_ENCODERS_PATH = "./models/text_encoders/";
const char* UNET_PATH = "./models/unet/";
const char* UPSCALES_PATH = "./models/upscale_models/";
const char* VAES_PATH = "./models/vae/";


const char* LIST_RESOLUTIONS_STR[] = {"64", "128", "192", "256", "320", "384", "448", "512", "576", "640", "704", "768", "832", "896", "960", "1024", "1088", "1152", "1216", "1280", NULL};
const size_t LIST_RESOLUTIONS_STR_COUNT = sizeof(LIST_RESOLUTIONS_STR) / sizeof(LIST_RESOLUTIONS_STR[0]);

const char* LIST_STEPS_STR[] = {"1", "2", "4", "8", "12", "16", "20", "24", "30", "36", "42", "50", "60", NULL};
const size_t LIST_STEPS_STR_COUNT = sizeof(LIST_STEPS_STR) / sizeof(LIST_STEPS_STR[0]);

const char* LIST_SAMPLES[] = {"euler", "euler_a", "heun", "dpm2", "dpm++2s_a", "dpm++2m", "dpm++2mv2", "ipndm", "ipndm_v", "lcm", NULL};
const size_t LIST_SAMPLES_COUNT = sizeof(LIST_SAMPLES) / sizeof(LIST_SAMPLES[0]);

const char* LIST_SCHEDULES[] = {"discrete", "karras", "exponential", "ays", "gits", NULL};
const size_t LIST_SCHEDULES_COUNT = sizeof(LIST_SCHEDULES) / sizeof(LIST_SCHEDULES[0]);


const char* POSITIVE_PROMPT = "A photo of a redhead woman standing under neon lights at night, wearing an elegant white dress. The scene is set on a rainy city street with a glossy, wet floor reflecting vibrant neon signs in red, blue, and purple. Cinematic lighting, soft shadows, and intricate fabric texture. Photorealistic, ultra high resolution, bokeh background, 85mm lens, shallow depth of field.";
const char* NEGATIVE_PROMPT = "(low quality, worst quality:1.5), extra fingers, mutated hands, ((poorly drawn hands)), ((poorly drawn face)), ((bad anatomy)), (((bad proportions))), ((extra limbs)), cloned face, (((disfigured))), out of frame, ugly, extra limbs, (bad anatomy), gross proportions, (malformed limbs), ((missing arms)), ((missing legs)), (((extra arms))), (((extra legs)))";
const char* OPTIONAL_ITEMS = "None";
const char* DEFAULT_IMG_PATH = "./resources/example.png";
const int DEFAULT_MODELS = 0;
const int DEFAULT_SAMPLE = 5;
const int DEFAULT_SCHEDULE = 1;
const int DEFAULT_N_STEPS = 7;
const int DEFAULT_SIZE = 7;
const int DEFAULT_BATCH_SIZE = 0;
const int DEFAULT_OPT_VRAM = 0;
const double DEFAULT_CFG = 6;
const double DEFAULT_DENOISE = 0.75;
const double DEFAULT_SEED = -1;
const double DEFAULT_RP_UPSCALE = 1;

