#include <stdio.h>

#include "TcpServer.h"

int main(int argc, char ** argv)
{
    CTcpServer *s = new CTcpServer(9898);
    s->start();
    return 0;
}
