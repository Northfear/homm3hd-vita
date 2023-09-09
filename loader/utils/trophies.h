#ifndef __TROPHIES_H__
#define __TROPHIES_H__

#ifdef __cplusplus
extern "C" {
#endif

int trophies_init();
void trophies_unlock(uint32_t id);
uint8_t trophies_is_unlocked(uint32_t id);

#ifdef __cplusplus
}
#endif
#endif
