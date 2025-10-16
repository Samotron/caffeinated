#ifndef FLUX_H
#define FLUX_H

int cmd_flux(int temp);
int cmd_flux_auto(volatile int *running);
int cmd_flux_reset(void);

#endif
