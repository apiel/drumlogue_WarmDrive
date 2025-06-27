
#include "unit.h"  // Note: Include common definitions for all units

#include "Effect.h"

static Effect s_effect;
static unit_runtime_desc_t s_runtime;

__unit_callback int8_t unit_init(const unit_runtime_desc_t * d) {
  if (!d || d->target != unit_header.target || !UNIT_API_IS_COMPAT(d->api)) return k_unit_err_target;
  s_runtime = *d;
  return s_effect.Init(d);
}

__unit_callback void unit_teardown() { s_effect.Reset(); }
__unit_callback void unit_reset() { s_effect.Reset(); }
__unit_callback void unit_resume() {}
__unit_callback void unit_suspend() {}
__unit_callback void unit_render(const float * in, float * out, uint32_t frames) {
  s_effect.Process(in, out, frames);
}
__unit_callback void unit_set_param_value(uint8_t id, int32_t value) { s_effect.setParameter(id, value); }
__unit_callback int32_t unit_get_param_value(uint8_t id) { return s_effect.getParameterValue(id); }
__unit_callback const char * unit_get_param_str_value(uint8_t id, int32_t value) {
  return s_effect.getParameterStrValue(id, value);
}
__unit_callback const uint8_t * unit_get_param_bmp_value(uint8_t, int32_t) { return nullptr; }
__unit_callback void unit_set_tempo(uint32_t) {}
__unit_callback void unit_load_preset(uint8_t) {}
__unit_callback uint8_t unit_get_preset_index() { return 0; }
__unit_callback const char * unit_get_preset_name(uint8_t) { return "Default"; }
