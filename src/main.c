#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "basis.h"
#include "grain.h"
#include "mpi.h"
#include "utils.h"

int main(int argc, char **argv)
{
    init_mpi(&argc, &argv);
    if (mpi.rank == 0) {
        init_gwclock();
    }

    // xtry(MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN));

    double t = get_gwclock();
    cpp_main(atoi(argv[1]));
    printf0("time_all=%.17g\n", get_gwclock() - t);

    MPI_Finalize();
    return 0;
}
