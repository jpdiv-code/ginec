#include "vm.h"

#include <stdbool.h>

#include "defs.h"
#include "opcode.h"

o_int32_t vm_exec(
    cartridge_t cartridge
) {
    uint32_t ip;
    int32_t opc;
    int32_t* mem;
    o_int32_t result;
    bool halt;

    mem = cartridge.mem;
    result.value = 0;
    result.err = NULL;
    halt = false;
    while (!halt)
    {
        ip = mem[0];
        opc = mem[ip];
        switch (opc)
        {
            case OPC_NOP:
            break;

            default:
                result.err =
                    err_new_runtime(NULL, cartridge, ip,
                        err_new_unknown_opcode(NULL, cartridge, opc, ip, NULL));
                halt = true;
            break;
        }
    }

    return result;
}
