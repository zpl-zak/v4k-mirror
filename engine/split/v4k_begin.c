#define do_threadlock(mutexptr) \
    for( int init_ = !!(mutexptr) || (thread_mutex_init( (mutexptr) = CALLOC(1, sizeof(thread_mutex_t)) ), 1); init_; init_ = 0) \
    for( int lock_ = (thread_mutex_lock( mutexptr ), 1); lock_; lock_ = (thread_mutex_unlock( mutexptr ), 0) )
