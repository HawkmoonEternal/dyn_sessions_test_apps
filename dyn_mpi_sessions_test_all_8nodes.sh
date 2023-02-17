echo "RUNNING TEST='VERSION 2A', MODE='i+', start_procs=8, end_procs=32 procs, delta_procs=8\n"

prterun -np 8 --display map  --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8 -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2a_release -c 120 -l 64 -m i+ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2A', MODE='i-', start_procs=32, end_procs=8 procs, delta_procs=8\n"

prterun -np 64 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2a_release -c 120 -l 1 -m i_ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2A', MODE='s+', start_procs=8, end_procs=32 procs, delta_procs=8,16,24\n"

prterun -np 8 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2a_release -c 200 -l 64 -m s+ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2A', MODE='s_', start_procs=32, end_procs=8 procs, delta_procs=8,16,24\n"

prterun -np 64 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2a_release -c 250 -l 1 -m s_ -n 8 -f 10 -b 0






echo "RUNNING TEST='VERSION 2A_NB', MODE='i+', start_procs=8, end_procs=32 procs, delta_procs=8\n"

prterun -np 8 --display map  --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8 -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2a_nb_release -c 120 -l 64 -m i+ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2A_NB', MODE='i-', start_procs=32, end_procs=8 procs, delta_procs=8\n"

prterun -np 64 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2a_nb_release -c 120 -l 1 -m i_ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2A_NB', MODE='s+', start_procs=8, end_procs=32 procs, delta_procs=8,16,24\n"

prterun -np 8 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2a_nb_release -c 200 -l 64 -m s+ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2A_NB', MODE='s_', start_procs=32, end_procs=8 procs, delta_procs=8,16,24\n"

prterun -np 64 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8 -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2a_nb_release -c 250 -l 1 -m s_ -n 8 -f 10 -b 0



echo "RUNNING TEST='VERSION 2B', MODE='i+', start_procs=8, end_procs=32 procs, delta_procs=8\n"

prterun -np 8 --display map  --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8 -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2b_release -c 120 -l 64 -m i+ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2B', MODE='i-', start_procs=32, end_procs=8 procs, delta_procs=8\n"

prterun -np 64 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2b_release -c 120 -l 1 -m i_ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2B', MODE='s+', start_procs=8, end_procs=32 procs, delta_procs=8,16,24\n"

prterun -np 8 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2b_release -c 200 -l 64 -m s+ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2B', MODE='s_', start_procs=32, end_procs=8 procs, delta_procs=8,16,24\n"

prterun -np 64 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2b_release -c 250 -l 1 -m s_ -n 8 -f 10 -b 0



echo "RUNNING TEST='VERSION 2B_NB', MODE='i+', start_procs=8, end_procs=32 procs, delta_procs=8\n"

prterun -np 8 --display map  --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8 -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2b_nb_release -c 120 -l 64 -m i+ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2B_NB', MODE='i-', start_procs=32, end_procs=8 procs, delta_procs=8\n"

prterun -np 64 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2b_nb_release -c 120 -l 1 -m i_ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2B_NB', MODE='s+', start_procs=8, end_procs=32 procs, delta_procs=8,16,24\n"

prterun -np 8 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2b_nb_release -c 200 -l 64 -m s+ -n 8 -f 10 -b 0

echo "RUNNING TEST='VERSION 2B_NB', MODE='s_', start_procs=32, end_procs=8 procs, delta_procs=8,16,24\n"

prterun -np 64 --display map --mca btl_tcp_if_include eth0 --host n01:8,n02:8,n03:8,n04:8,n05:8,n06:8,n07:8,n08:8  -x LD_LIBRARY_PATH -x DYNMPI_BASE $DYNMPI_BASE/build/examples/tests/dyn_sessions_test_apps/build/DynMPISessions_v2b_nb_release -c 250 -l 1 -m s_ -n 8 -f 10 -b 0



