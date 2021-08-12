#include <kernel.h>

extern void	tTask_start_task(intptr_t exinf);
extern void	tTask_start_exception(TEXPTN texptn, intptr_t exinf);

extern void tInitializeRoutine_start(intptr_t exinf);
extern void tTerminateRoutine_start(intptr_t exinf);

extern void tISR_start(intptr_t exinf);

extern void tCyclicHandler_start(intptr_t exinf);
extern void tAlarmHandler_start(intptr_t exinf);

