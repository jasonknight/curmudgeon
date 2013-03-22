#include <curmudgeon.h>
#include <handlers.h>
// Get a single {{RESOURCE}}
int get_{{RESOURCE}}(event_t ** src) {
    /* 
     * I don't know if event should be changeable,
     * ideally it shouldn't be.  
     */
    event_t * event = *src;
    // We need app here so that we can run queries.
    // it's a trade off with a silver bullet.
    //
    // To simplify the library, you call functions
    // with &app which keeps track of things like
    // your database connection etc..
    curmudgeon_t * app = event->cur;
    #include "_before.c"
    /* You code begins here. */

    /* Your code should end here. */
    #include "_after.c"
    return CUR_OK;
}
// Get all resources, paginated
int get_{{RESOURCES}} (event_t ** src) {
    event_t * event = *src;
    curmudgeon_t * app = event->cur;
    #include "_before.c"
    /* You code begins here. */

    /* Your code should end here. */
    #include "_after.c"
    return CUR_OK;
}
// Save a single resource
int post_{{RESOURCE}}(event_t * src) {
    event_t * event = *src;
    curmudgeon_t * app = event->cur;
    #include "_before.c"
    /* You code begins here. */

    /* Your code should end here. */
    #include "_after.c"
    return CUR_OK;
}
// Delete a single resource
int delete_{{RESOURCE}}(event_t * src) {
    event_t * event = *src;
    curmudgeon_t * app = event->cur;
    #include "_before.c"
    /* You code begins here. */

    /* Your code should end here. */
    #include "_after.c"
    return CUR_OK;
}
