/*
 * Copyright (c) 2004-2006 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2006      Cisco Systems, Inc.  All rights reserved.
 *
 * Sample MPI "hello world" application in C
 */

#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <linux/limits.h>
#include <time.h>
#include <stdbool.h>
#include "mpi.h"
#include <unistd.h>
#include <signal.h>

#define BENCH_INCRIMENTAL 0
#define BENCH_STEP 1

/* Simulation parameters */
const unsigned long long PROBLEM_SIZE = 100000000;

int ITER_MAX = 200;
int proc_limit = 64;
int proc_limit_up = 64;
int proc_limit_down = 1;
int num_delta = 8;
int rc_frequency = 10;
int mode_type = MPI_PSETOP_ADD;
int mode_num = BENCH_INCRIMENTAL;
int blocking = 0;

int cur_type, cur_num_delta, check_rc; 


extern char *optarg;
extern int optind, opterr, optout, optopt;

char * itoa(int num){
    int length = snprintf( NULL, 0, "%d", num);
    char *string = (char *) malloc( length + 1 );   
    snprintf( string, length + 1, "%d", num);
    return string;
}


int set_mode(const char *c_mode){
    

    if(strlen(c_mode) != 2){
        return -1;
    }

    if(c_mode[0] == 'i'){
        mode_num = BENCH_INCRIMENTAL;
    }else if(c_mode[0] == 's'){
        mode_num = BENCH_STEP;
    }else{
        return -1;
    }
    cur_num_delta = num_delta;
    

    if(c_mode[1] == '+'){
        mode_type = MPI_PSETOP_ADD;
        proc_limit_up = proc_limit;
    }else if(c_mode[1] == '_'){
        mode_type = MPI_PSETOP_SUB;
        proc_limit_down = proc_limit;
    }else{
        return -1;
    }
    cur_type = mode_type;

    check_rc = 1;

    return 0;
}

void eval_parameters(int current_size, int *cur_type, int *cur_num_delta, int *check_rc){
    switch(mode_num){
        case BENCH_STEP:       
            *cur_type = *cur_type == MPI_PSETOP_ADD ? MPI_PSETOP_SUB : MPI_PSETOP_ADD;
            *cur_num_delta = *cur_type == mode_type ? *cur_num_delta + num_delta: *cur_num_delta;
            *check_rc = *cur_num_delta <= 0 ? false : *cur_type == MPI_PSETOP_ADD ? current_size + *cur_num_delta <= proc_limit_up : (current_size - *cur_num_delta >= proc_limit_down);
            break;
        case BENCH_INCRIMENTAL:
            *check_rc = mode_type == MPI_PSETOP_ADD ? current_size + *cur_num_delta <= proc_limit_up : (current_size - *cur_num_delta >= proc_limit_down);
            break;
        default:
            break;            
    }
}

int send_application_data(MPI_Comm comm, int rank, int cur_iter, int cur_type, int cur_num_delta, int check_rc){
    int rc;
    int buf[4];

    if(rank == 0){
        buf[0] = cur_iter;
        buf[1] = cur_type;
        buf[2] = cur_num_delta;
        buf[3] = check_rc;
    }

    rc = MPI_Bcast(buf, 4, MPI_INT, 0, comm);

    return rc;

}

int recv_application_data(MPI_Comm comm, int *cur_iter, int *cur_type, int *cur_num_delta, int *check_rc){
    int rc;
    int buf[4];

    if(MPI_SUCCESS != (rc = MPI_Bcast(buf, 4, MPI_INT, 0, comm))){
        return rc;
    }

    *cur_iter = buf[0];
    *cur_type = buf[1];
    *cur_num_delta = buf[2];
    *check_rc = buf[3];

    return MPI_SUCCESS;
}



int parse_arguments(int argc, char** argv){

    opterr = 0;

    int c;
    char *mode_s = NULL;


    while ((c = getopt (argc, (char**)argv, "c:m:l:n:f:b:")) != -1){
      switch (c)
        {
        case 'b':
            blocking = atoi(optarg);
            break;
        case 'c':
            ITER_MAX = atoi(optarg);
            break;
        case 'l':
            proc_limit = atoi(optarg);
            break;
        case 'n':
            num_delta = atoi(optarg);
            break;
        case 'f':
            rc_frequency = atoi(optarg);
            break;
        case 'm':
            mode_s = optarg;
            break;
        case '?':
          if (optopt == 'c' || optopt == 'm' || optopt == 'l' || optopt == 'n' || optopt == 'f' || optopt == 'b')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
          else
            fprintf (stderr,
                     "Unknown option character `\\x%x'.\n",
                     optopt);
          return 1;
        default:
            printf("Bad Parameter: Abort!\n");
          abort ();
        }
    }
    if(NULL != mode_s){
        set_mode(mode_s);
    }
    return 0;
}

void free_string_array(char **array, int size){
    int i;
    if(0 == size){
        return;
    }
    for(i = 0; i < size; i++){
        free(array[i]);
    }
    free(array);
}


/* create a communicator from the given process set */
#pragma region
int comm_create_from_pset(MPI_Session session_handle, char *pset_name, MPI_Comm *comm, int *num_procs, int *rank){
    int rc;
    MPI_Group wgroup = MPI_GROUP_NULL;

    /* create a group from pset */
    MPI_Group_from_session_pset (session_handle, pset_name, &wgroup);

    /* create a communicator from group */
    MPI_Comm_create_from_group(wgroup, "mpi.forum.example", MPI_INFO_NULL, MPI_ERRORS_RETURN, comm);

    if(MPI_COMM_NULL == *comm){
        printf("comm_from_group returned MPI_COMM_NULL\n");
    }

    /* get the size of the new communicator */
    MPI_Comm_size(*comm, num_procs);

    /* get the rank of this process in the new communicator */
    MPI_Comm_rank(*comm, rank);

    MPI_Group_free(&wgroup);
    return MPI_SUCCESS;
}

#pragma endregion
 
int work_step(int start_index, int end_index){
    unsigned long long  n;
    int i = 0;
    double res = .0;
    double a = 1.57, b = 20.1113, c = 5.4200102;
    double d = 5.223, e = 1.89, f = 22.223;
    double t, x, y;
    for(n = start_index; n < end_index; n++){
        t = (double) n;
        x = a*t*t + b*t + c;
        y = d*t*t + e*t + f;

        if(x > 42.42 && x < 42.4242 && y > 42.42 && y < 42.4242){
            i++;
        }
    }
    return i;
    
}

void rebalance_step(int num_procs, int my_work_rank, long long unsigned int*start_index, long long unsigned int*end_index){
    unsigned long long chunk_size = PROBLEM_SIZE / num_procs;
    *start_index = chunk_size * my_work_rank;
    *end_index   = (my_work_rank == num_procs - 1)   ? PROBLEM_SIZE  : (my_work_rank + 1) * chunk_size;
}


int main(int argc, char* argv[])
{
    int rc, flag, num_procs, rank, op;
    int cur_iter = 0, iters_since_last_change = 0;
    unsigned long long start_index;
    unsigned long long end_index;
    bool dynamic_proc = false, primary_proc = false, terminate = false;

    int rc_type, noutput;

    char _keys[][MPI_MAX_INFO_KEY] = {"mpi_dyn", "mpi_primary", "next_main_pset", "mpi_included"}; // Is it a PSet representing dynamically added resources?; Am I the primary process of this PSet?; Name of the main PSet to use; Am I included in this PSet? 
    char *keys[4] = {_keys[0], _keys[1], _keys[2], _keys[3]};
    char boolean_string[6];
    char *str;
    char *str_ptr;
    char mpi_world_pset[] = "mpi://WORLD";
    char main_pset[MPI_MAX_PSET_NAME_LEN];
    char delta_pset[MPI_MAX_PSET_NAME_LEN];
    char old_main_pset[MPI_MAX_PSET_NAME_LEN];
    char **input_psets = NULL, **output_psets = NULL;

    MPI_Comm comm = MPI_COMM_NULL;
    MPI_Session session_handle = MPI_SESSION_NULL;
    MPI_Info info = MPI_INFO_NULL;

    /* Parse the command line arguments*/
	parse_arguments(argc, argv);

    /* mpi://WORLD is the initial main PSet */
    strcpy(main_pset, "mpi://WORLD");
    strcpy(delta_pset, "");

    /* Initialize the session */
    MPI_Session_init(MPI_INFO_NULL, MPI_ERRORS_RETURN, &session_handle);

    /* Get the data from our mpi://WORLD pset */
    MPI_Session_get_pset_data (session_handle, mpi_world_pset, mpi_world_pset, (char **) keys, 2, true, &info);

    /* get value for the 'mpi_dyn' key -> if true, this process was added dynamically */
    MPI_Info_get(info, "mpi_dyn", 6, boolean_string, &flag);
    
    /* if mpi://WORLD is a dynamic PSet retrieve the name of the main PSet stored on mpi://WORLD */
    if(flag && 0 == strcmp(boolean_string, "True")){
        
        MPI_Info_free(&info);
        
        dynamic_proc = true;
        
        MPI_Session_get_pset_data (session_handle, mpi_world_pset, mpi_world_pset, (char **) &keys[2], 1, true, &info);
        MPI_Info_get(info, "next_main_pset", MPI_MAX_PSET_NAME_LEN, main_pset, &flag);

        if(!flag){
            printf("No 'next_main_pset' was provided for dynamic process. Terminate.\n");
            MPI_Session_finalize(&session_handle);
            return rc;
        }
        MPI_Info_free(&info);

        /* Get PSet data stored on main PSet */
        MPI_Session_get_pset_data (session_handle, mpi_world_pset, main_pset, (char **) &keys[1], 1, true, &info);
    }

    /* Check if this proc is the 'primary process' of the main PSet */
    MPI_Info_get(info, "mpi_primary", 6, boolean_string, &flag);

    if(flag && 0 == strcmp(boolean_string, "True")){
        primary_proc = true;
    }

    MPI_Info_free(&info);

    /* create a communicator from the main PSet*/
    comm_create_from_pset(session_handle, main_pset, &comm, &num_procs, &rank);

    /* If this is a dynamic process, it needs to recv the current application data */
    if(dynamic_proc){
        recv_application_data(comm, &cur_iter, &cur_type, &cur_num_delta, &check_rc);

        /* If this process is the primary process it needs to finalize the resource change */
        if(primary_proc){
            MPI_Session_dyn_finalize_psetop(session_handle, main_pset);
        }
    }

    
    /* MAIN APPLICATION LOOP */
    while(cur_iter++ < ITER_MAX){

        if(rank == 0){
            printf("Start of iteration %d with %d processes in PSet %s\n", cur_iter, num_procs, main_pset);
        }
        /* App-specific data redistribution */
        rebalance_step(num_procs, rank, &start_index, &end_index);

        /* App-specific work step */
        work_step(start_index, end_index);
        
        MPI_Barrier(comm);

        /* Resource Change Step */
        if(check_rc && ++iters_since_last_change >= rc_frequency){

            /* Check for a resource change related to the main PSet (e.g. from RM) */
            MPI_Session_dyn_v2a_query_psetop(session_handle, main_pset, main_pset, &rc_type, &output_psets, &noutput);

            if(MPI_PSETOP_NULL == rc_type){
                /* In this example the 'primary process' requests set operations */
                if(primary_proc){
                    MPI_Info_create(&info);

                    str = itoa(cur_num_delta);
                    MPI_Info_set(info, "mpi.op_info.info.num_procs", str);
                    free(str);

                    noutput = 0;
                    input_psets = (char **) malloc(2 * sizeof(char*));
                    input_psets[0] = strdup(main_pset);
                    /* Request ADD / SUB operation*/
                    op = cur_type == MPI_PSETOP_ADD ? MPI_PSETOP_GROW : MPI_PSETOP_SHRINK;
                    MPI_Session_dyn_v2a_psetop(session_handle, &op, input_psets, 1, &output_psets, &noutput, info);

                    MPI_Info_free(&info);
                    
                    if(MPI_PSETOP_NULL != op){
                        /* Publish the name of the new main PSet on the delta Pset */
                        MPI_Info_create(&info);
                        MPI_Info_set(info, "next_main_pset", output_psets[1]);
                        MPI_Session_set_pset_data(session_handle, output_psets[0], info);
                        MPI_Info_free(&info);
                    }

                    free_string_array(input_psets, 2);
                    free_string_array(output_psets, noutput);
                }

                /* Now again query for the Set operation info */      
                MPI_Session_dyn_v2a_query_psetop(session_handle, main_pset, main_pset, &rc_type, &output_psets, &noutput);
            }
            usleep(500000);
            /* Continue work if no new PSETOP was found */
            if(MPI_PSETOP_NULL == rc_type || 0 == strcmp(delta_pset, output_psets[0])){
                iters_since_last_change = 0;
                continue;
            }

            strcpy(delta_pset, output_psets[0]);

            /* Retrieve data stored on the delta PSet */
            MPI_Session_get_pset_data (session_handle, main_pset, output_psets[0], (char **) &keys[1], 3, true, &info);

            /* Is proc included in the delta PSet? */
            MPI_Info_get(info, "mpi_included", 6, boolean_string, &flag);

            /* This process is included in the delta PSet => it needs to terminate */
            if(0 == strcmp(boolean_string, "True")){
                terminate = true;
            }

            /* Get the name of the new main PSet */
            strcpy(old_main_pset, main_pset);
            MPI_Info_get(info, "next_main_pset", MPI_MAX_PSET_NAME_LEN, main_pset, &flag); 
            
            if(!flag){
                printf("could not find next_main_pset on PSet %s. This should never happen! Terminate.\n", main_pset);
            }

            MPI_Info_free(&info);

            /* Is this proc the primary process of the new main PSet? Primary proc could have changed! */
            MPI_Session_get_pset_data (session_handle, old_main_pset, main_pset, (char **) &keys[1], 1, true, &info);
            MPI_Info_get(info, "mpi_primary", 6, boolean_string, &flag);
            primary_proc = (0 == strcmp(boolean_string, "True"));
            MPI_Info_free(&info);

            /* This process is included in the delta PSet => it need to terminate */
            if(terminate){
                break;
            }

            /* Disconnect from the old communicator */
            MPI_Comm_disconnect(&comm);

            /* create a cnew ommunicator from the new main PSet*/
            comm_create_from_pset(session_handle, main_pset, &comm, &num_procs, &rank);

            /* Update and send the application parameters */
            eval_parameters(num_procs, &cur_type, &cur_num_delta, &check_rc);
            send_application_data(comm, rank, cur_iter, cur_type, cur_num_delta, check_rc);

            if(primary_proc){
                MPI_Session_dyn_finalize_psetop(session_handle, old_main_pset);
            }

            /* Reset the counter since last resource change */
            iters_since_last_change = 0;
        }
        

    } /* END OF MAIN APPLICATION LOOP */

    /* Disconnect from the communicator */
    MPI_Comm_disconnect(&comm);

    /* Finalize the sesison */
    MPI_Session_finalize(&session_handle);

    if(0 == rank){
        printf("finished becnhmark sucessfully!\n");
    }

    return 0;
}
