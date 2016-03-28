#include "policy.h"
#include "quorum.h"

int main(int argc, char **argv) {
    Policy *policy = new Quorum();
    return 0;
}
