#include "kernel.h"
#include "t_syslog.h"

#define dumfun( func )  \
    extern void func();        \
    void func(){ syslog( LOG_NOTICE, "dummy: " #func " called" ); }

/*** ダミー関数 ***/
dumfun( _kill )
dumfun( _getpid )
dumfun( _exit )
dumfun( _sbrk )
dumfun( _fstat )
dumfun( _isatty )
dumfun( _gettimeofday )
dumfun( _fini )
// dumfun( _write )
dumfun( _close )
dumfun( _lseek )
dumfun( _read )
// dumfun( mrb_open_allocf )

int _write( int fd, char *buf, size_t sz )
{
#define BUF_LEN 32
    int i;

    char buf2[ BUF_LEN ];
    for( i = 0; i < BUF_LEN - 1; i++ ){
        buf2[i] = buf[i];
    }
    buf2[ sz < BUF_LEN ? sz : BUF_LEN - 1] = '\0';

    syslog( LOG_NOTICE, "_write: %s", buf2);
    dly_tsk( 100 );

    return i;
}
