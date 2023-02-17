/*
 * Copyright (c) 2004-2006 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2006      Cisco Systems, Inc.  All rights reserved.
 *
 * Sample MPI "hello world" application in C
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
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

int num_procs;
int my_work_rank;

int rank; 

extern char *optarg;
extern int optind, opterr, optout, optopt;

char * itoa(const char *prefix, int num){

    int length = snprintf( NULL, 0, "%d", num) + strlen(prefix);
    char *string = (char *) malloc( length + 1 );
    strcpy(string, prefix);   
    snprintf(string + strlen(prefix), length + 1, "%d", num);
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
int comm_create_from_pset(MPI_Session session, char *pset_name, MPI_Comm *comm, int *num_procs, int *rank){
    int rc;
    MPI_Group wgroup = MPI_GROUP_NULL;

    /* create a group from pset */
    if(MPI_SUCCESS != (rc = MPI_Group_from_session_pset (session, pset_name, &wgroup))){
        printf("MPI_comm_create_from_group failed with rc = %d\n", rc);
        return rc;        
    }

    /* create a communicator from group */
    if(MPI_SUCCESS != (rc = MPI_Comm_create_from_group(wgroup, "mpi.forum.example", MPI_INFO_NULL, MPI_ERRORS_RETURN, comm))){
        printf("MPI_comm_create_from_group failed with rc = %d\n", rc);
        return rc;
    }

    MPI_Group_size(wgroup, num_procs);

    if(MPI_COMM_NULL == *comm){
        printf("comm_from_group returned MPI_COMM_NULL\n");
    }

    /* get the size of the new communicator */
    if(MPI_SUCCESS != (rc = MPI_Comm_size(*comm, num_procs))){
        printf("MPI_comm_size failed with rc = %d\n", rc);
        return rc;
    }

    /* get the rank of this process in the new communicator */
    if(MPI_SUCCESS != (rc = MPI_Comm_rank(*comm, rank))){
        printf("MPI_comm_rank failed with rc = %d\n", rc);
        return rc;
    }


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
    int pset_name_len = MPI_MAX_PSET_NAME_LEN;
    int rc, flag, num_procs, rank;
    int cur_iter = 0, iters_since_last_change = 0, rc_count = 0;
    unsigned long long start_index;
    unsigned long long end_index;
    bool dynamic_proc = false, primary_proc = false, terminate = false;

    int rc_type, noutput;

    char _keys[][MPI_MAX_INFO_KEY] = {"next_main_pset"}; // Name of the main PSet to use 
    char *keys[1] = {_keys[0]};
    char boolean_string[6];
    char *str;
    char *str_ptr;
    char mpi_world_pset[] = "mpi://WORLD";
    char main_pset[MPI_MAX_PSET_NAME_LEN];
    char old_main_pset[MPI_MAX_PSET_NAME_LEN];
    char delta_pset[MPI_MAX_PSET_NAME_LEN];
    char new_delta_pset[MPI_MAX_PSET_NAME_LEN];
    char **pset_array = NULL;

    MPI_Comm comm = MPI_COMM_NULL;
    MPI_Session session = MPI_SESSION_NULL;
    MPI_Info info = MPI_INFO_NULL;
    MPI_RC_handle rc_handle = MPI_RC_HANDLE_NULL;
    MPI_Request request = MPI_REQUEST_NULL;
    MPI_Status status;


    /* Parse the command line arguments*/
	parse_arguments(argc, argv);

    /* mpi://WORLD is the initial main PSet */
    strcpy(main_pset, "mpi://WORLD");
    strcpy(delta_pset, "");

    /* Initialize the session */
    MPI_Session_init(MPI_INFO_NULL, MPI_ERRORS_ARE_FATAL, &session);

    /* Get the some data from our mpi://WORLD pset */
    MPI_Session_get_pset_info (session, main_pset, &info);

    /* get value for the 'mpi_dyn' key*/
    MPI_Info_get(info, "mpi_dyn", 6, boolean_string, &flag);

    
    /* if mpi://WORLD is a dynamic PSet retrieve the name of the main PSet stored on our mpi://WORLD */
    if(flag && 0 == strcmp(boolean_string, "True")){
        
        MPI_Info_free(&info);
        
        dynamic_proc = true;

        
        MPI_Session_get_pset_data_nb (session, mpi_world_pset, mpi_world_pset, (char **) &keys[0], 1, true, &info, &request);
        MPI_Wait(&request, &status);
        MPI_Info_get(info, "next_main_pset", MPI_MAX_PSET_NAME_LEN, main_pset, &flag);

        if(!flag){
            printf("No 'next_main_pset' was provided for dynamic process. Terminate.\n");
            MPI_Session_finalize(&session);
            return rc;
        }
        MPI_Info_free(&info);

        MPI_Session_get_pset_info (session, main_pset, &info);
    }

    /* Check if this proc is the 'primary process' of the main PSet */
    MPI_Info_get(info, "mpi_primary", 6, boolean_string, &flag);

    if(flag && 0 == strcmp(boolean_string, "True")){
        primary_proc = true;
    }

    MPI_Info_free(&info);

    /* create a communicator from the main PSet*/
    comm_create_from_pset(session, main_pset, &comm, &num_procs, &rank);

    /* If this is a dynamic process, it needs to recv the current application data */
    if(dynamic_proc){
        recv_application_data(comm, &cur_iter, &cur_type, &cur_num_delta, &check_rc);

        if(primary_proc){
            MPI_Session_dyn_finalize_psetop(session, main_pset);
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
        
        if(MPI_SUCCESS != (rc = MPI_Barrier(comm))){
            printf("MPI_Barrier failed for pset %s\n", main_pset);
        }

        /* Resource Change Step */
        if(check_rc && ++iters_since_last_change >= rc_frequency){

            rc_type = MPI_PSETOP_NULL;
            /* Check for a resource change related to the main PSet (e.g. from the RM) */
            MPI_Session_dyn_v2b_query_psetop_nb(session, main_pset, main_pset, &rc_handle, &request);

            MPI_Wait(&request, &status);

            if(MPI_RC_HANDLE_NULL != rc_handle){
                MPI_Session_dyn_v2b_rc_handle_get_op_type(session, rc_handle, 0, &rc_type);
            }
            
            if(MPI_PSETOP_NULL == rc_type){

                /* In this example the 'primary process' requests set operations */
                if(primary_proc){
                    MPI_Info_create(&info);
                    str = itoa("", cur_num_delta);
                    if(cur_type == MPI_PSETOP_ADD){
                        MPI_Info_set(info, "mpi_num_procs_add", str);
                    }else{
                        MPI_Info_set(info, "mpi_num_procs_sub", str);
                    }
                    free(str);

                    pset_array = (char **) malloc(3 * sizeof(char *));
                    pset_array[0] = strdup(main_pset);
                    pset_array[1] = itoa("app://delta_pset/", rc_count);
                    pset_array[2] = itoa("app://main_pset/", rc_count);

                    MPI_Session_dyn_v2b_rc_handle_create(session, &rc_handle);

                    MPI_Session_dyn_v2b_rc_handle_add_op(session, cur_type, pset_array, 1, &pset_array[1], 1, info, rc_handle);
                    MPI_Info_free(&info);

                    MPI_Session_dyn_v2b_rc_handle_add_op(session, cur_type == MPI_PSETOP_ADD ? MPI_PSETOP_UNION : MPI_PSETOP_DIFFERENCE, pset_array, 2, &pset_array[2], 1, MPI_INFO_NULL, rc_handle);

                    MPI_Info_create(&info);
                    MPI_Info_set(info, "next_main_pset", pset_array[2]);
                    MPI_Session_dyn_v2b_rc_handle_add_pset_info(session, rc_handle, pset_array[1], info);
                    MPI_Info_free(&info);

                    MPI_Session_dyn_v2b_psetop_nb(session, rc_handle, &request);
                    MPI_Wait(&request, &status);

                    MPI_Session_dyn_v2b_rc_handle_free(session, &rc_handle);
                    free_string_array(pset_array, 3);
                    
                }
                
                /* Now again query for the Set operation info */      
                MPI_Session_dyn_v2b_query_psetop_nb(session, main_pset, main_pset, &rc_handle, &request);

                MPI_Wait(&request, &status);

                if(MPI_RC_HANDLE_NULL != rc_handle){
                    MPI_Session_dyn_v2b_rc_handle_get_op_type(session, rc_handle, 0, &rc_type);
                    MPI_Session_dyn_v2b_rc_handle_get_output_pset(session, rc_handle, 0, 0, &pset_name_len, new_delta_pset);
                    MPI_Session_dyn_v2b_rc_handle_free(session, &rc_handle);
                }
            }

            if(MPI_PSETOP_NULL == rc_type || 0 == strcmp(delta_pset, new_delta_pset)){
                iters_since_last_change = 0;
                continue;
            }

            rc_count ++;
            strcpy(delta_pset, new_delta_pset);
            
            /* Retrieve data stored on the delta PSet */
            MPI_Session_get_pset_info (session, new_delta_pset, &info);
            MPI_Info_get(info, "mpi_included", 6, boolean_string, &flag);
            if(0 == strcmp(boolean_string, "True")){
                terminate = true;
            }

            strcpy(old_main_pset, main_pset);
            MPI_Session_get_pset_data_nb(session, main_pset, new_delta_pset, (char **) &keys[0], 1, true, &info, &request); 
            MPI_Wait(&request, &status);           
            MPI_Info_get(info, "next_main_pset", MPI_MAX_PSET_NAME_LEN, main_pset, &flag);
            if(!flag){
                printf("MPI_Info_get failed\n");
                MPI_Session_finalize(&session);
                return rc;
            }
            MPI_Info_free(&info);

            MPI_Session_get_pset_info (session, main_pset, &info);
            MPI_Info_get(info, "mpi_primary", 6, boolean_string, &flag);
            MPI_Info_free(&info);

            /* Primary proc could have changed */
            primary_proc = (0 == strcmp(boolean_string, "True"));

            if(terminate){
                break;
            }

            /* Disconnect from the old communicator */
            MPI_Comm_disconnect(&comm);
            
            /* create a cnew ommunicator from the new main PSet*/
            comm_create_from_pset(session, main_pset, &comm, &num_procs, &rank);

            /* Update and send the application parameters */
            eval_parameters(num_procs, &cur_type, &cur_num_delta, &check_rc);
            send_application_data(comm, rank, cur_iter, cur_type, cur_num_delta, check_rc);

            if(primary_proc){
                MPI_Session_dyn_finalize_psetop(session, old_main_pset);
            }

            /* Reset the counter since last resource change */
            iters_since_last_change = 0;

        }
        

    } /* END OF MAIN APPLICATION LOOP */

    /* Disconnect from the communicator */
    MPI_Comm_disconnect(&comm);

    /* Finalize the sesison */
    MPI_Session_finalize(&session);

    if(0 == rank){
        printf("finished becnhmark sucessfully!\n");
    }

    //printf("Rank %d finished its work with great success! Bye Bye! :)\n", rank);
    
    return 0;
}
