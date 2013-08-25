#include "MessageQueue.h"
#include "ThreadPool.h"

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

// -----------------------------------------------------------------------------

void test_MessageQueue( );
void test_ThreadPool( );

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    // test_MessageQueue( );
    test_ThreadPool( );

    return 0;
}
