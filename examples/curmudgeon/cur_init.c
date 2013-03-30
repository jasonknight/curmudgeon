#include <curmudgeon.h>

curmudgeon_t *  app              = NULL;
int             number_of_events = 5;
if ( cur_init( &app, number_of_events ) == CUR_OK ) {
    ...
}
