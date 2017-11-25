#include <stdio.h>

#include "TcpServer.h"

int main(int argc, char ** argv)
{
    CTcpServer *s = new CTcpServer(9898);
    s->get_model()->set_timeout(1000);

    if (s->start() == 0)
        s->get_model()->start();
    return 0;
}
