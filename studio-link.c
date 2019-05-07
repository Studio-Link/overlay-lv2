#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <re/re.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "baresip.h"

#define STUDIOLINK_URI "http://studio-link.de/plugins/lv2/studio-link"

pthread_t tid;

struct session;
void effect_play(struct session *sess, float* const output0,
		float* const output1, unsigned long nframes);
void effect_src(struct session *sess, const float* const input0,
		const float* const input1, unsigned long nframes);
void effect_bypass(struct session *sess, float* const output0,
		float* const output1, const float* const input0,
		const float* const input1, unsigned long nframes);

struct session* effect_session_start(void);
int effect_session_stop(struct session *session);


typedef enum {
	AMP_INPUT0  = 0,
	AMP_INPUT1  = 1,
	AMP_OUTPUT0  = 2,
	AMP_OUTPUT1 = 3
} PortIndex;

typedef struct {
	// Port buffers
	const float* input0;
	const float* input1;
	float*       output0;
	float*       output1;
	struct session *sess;
} Amp;

bool running = false;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{

	Amp* amp = (Amp*)malloc(sizeof(Amp));

	if (!running) {

		(void)re_fprintf(stderr, "activate baresip v%s"
				" Copyright (C) 2010 - 2015"
				" Alfred E. Heggestad et al.\n",
				BARESIP_VERSION);
		(void)sys_coredump_set(true);
		libre_init();
		conf_configure(true);
		baresip_init(conf_config(), false);
		ua_init("baresip v" BARESIP_VERSION " (" ARCH "/" OS ")",
				true, true, true, false);
		conf_modules();
		pthread_create(&tid, NULL, (void*(*)(void*))&re_main, NULL);
		running = true;
	}

	amp->sess = effect_session_start();

	return (LV2_Handle)amp;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Amp* amp = (Amp*)instance;

	switch ((PortIndex)port) {
	case AMP_INPUT0:
		amp->input0 = (const float*)data;
		break;
	case AMP_INPUT1:
		amp->input1 = (const float*)data;
		break;
	case AMP_OUTPUT0:
		amp->output0 = (float*)data;
		break;
	case AMP_OUTPUT1:
		amp->output1 = (float*)data;
		break;
	}
}

static void
activate(LV2_Handle instance)
{
}

/**
   The `run()` method is the main process function of the plugin.  It processes
   a block of audio in the audio context.  Since this plugin is
   `lv2:hardRTCapable`, `run()` must be real-time safe, so blocking (e.g. with
   a mutex) or memory allocation are not allowed.
*/
static void
run(LV2_Handle instance, uint32_t n_samples)
{
	const Amp* amp = (const Amp*)instance;

	const float* const input0  = amp->input0;
	const float* const input1  = amp->input1;
	float* const       output0 = amp->output0;
	float* const       output1 = amp->output1;

	effect_src(amp->sess, input0, input1, n_samples);
	effect_play(amp->sess, output0, output1, n_samples);
	effect_bypass(amp->sess, output0, output1,
			input0, input1, n_samples);
}

static void
deactivate(LV2_Handle instance)
{
}

/**
   Destroy a plugin instance (counterpart to `instantiate()`).

   This method is in the ``instantiation'' threading class, so no other
   methods on this instance will be called concurrently with it.
*/
static void
cleanup(LV2_Handle instance)
{
	Amp* amp = (Amp*)instance;
	if (!effect_session_stop(amp->sess)) {
		//re_cancel();
		ua_stop_all(false);
		//(void)pthread_join(tid, NULL);
		sys_msleep(500);
		ua_close();
		re_cancel();
		conf_close();
		baresip_close();
		mod_close();
		libre_close();
		running = false;
	}

	free(instance);
}

static const void*
extension_data(const char* uri)
{
	return NULL;
}

static const LV2_Descriptor descriptor = {
	STUDIOLINK_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:  return &descriptor;
	default: return NULL;
	}
}
