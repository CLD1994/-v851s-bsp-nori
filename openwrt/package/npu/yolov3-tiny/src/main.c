/****************************************************************************
 *   Generated by ACUITY 5.24.0
 *
 *   Neural Network application project entry file
 ****************************************************************************/
/*-------------------------------------------
                Includes
-------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <vip_lite.h>

#define _BASETSD_H

#include "vnn_global.h"
#include "vnn_post_process.h"
#include "vnn_pre_process.h"

/*-------------------------------------------
        Macros and Variables
-------------------------------------------*/
const char *usage = "Usage:\nnbg_name input_data1 input_data2...";
/*-------------------------------------------
                  Functions
-------------------------------------------*/
#define BILLION 1000000000
static vip_uint64_t get_perf_count() {
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);

  return (vip_uint64_t)((vip_uint64_t)ts.tv_nsec +
                        (vip_uint64_t)ts.tv_sec * BILLION);
}

vip_status_e vnn_InitNetworkItem(vip_network_items **network_items, int argc,
                                 char **argv) {
  /*
   * argv0:   execute file
   * argv1:   nbg file
   * argv2~n: inputs n files
   */
  vip_status_e status = VIP_SUCCESS;
  vip_network_items *nnItems = NULL;
  const char *file_name = NULL;
  int input_num = 0, i = 0;
  char **inputs = NULL;
  int name_len = 0;

  file_name = (const char *)argv[1];
  input_num = argc - 2;
  if (input_num <= 0) {
    status = VIP_ERROR_INVALID_ARGUMENTS;
    goto final;
  }
  inputs = argv + 2;

  nnItems = (vip_network_items *)malloc(sizeof(vip_network_items));
  memset(nnItems, 0, sizeof(vip_network_items));

  name_len = strlen(file_name);
  if (name_len <= 0) {
    if (nnItems) {
      free(nnItems);
      nnItems = NULL;
    }
    status = VIP_ERROR_INVALID_ARGUMENTS;
    goto final;
  }
  nnItems->nbg_name = (char *)malloc(name_len + 1);
  memset(nnItems->nbg_name, 0, name_len + 1);
  strcpy(nnItems->nbg_name, file_name);
  nnItems->input_count = input_num;
  nnItems->input_names = (char **)malloc(sizeof(char *) * input_num);
  for (i = 0; i < input_num; i++) {
    nnItems->input_names[i] = inputs[i];
  }

  *network_items = nnItems;

final:
  return status;
}

static vip_status_e vnn_CreateNeuralNetwork(vip_network_items *network_items) {
  vip_status_e status = VIP_SUCCESS;
  vip_uint64_t tmsStart, tmsEnd;
  float msVal, usVal;

  tmsStart = get_perf_count();
  status =
      vip_create_network(network_items->nbg_name, 0,
                         VIP_CREATE_NETWORK_FROM_FILE, &network_items->network);
  _CHECK_STATUS(status, final);
  tmsEnd = get_perf_count();
  msVal = (float)(tmsEnd - tmsStart) / 1000000;
  usVal = (float)(tmsEnd - tmsStart) / 1000;
  printf("Create Neural Network: %.2fms or %.2fus\n", msVal, usVal);

final:
  return status;
}

static vip_status_e
vnn_PreProcessNeuralNetwork(vip_network_items *network_items) {
  vip_status_e status = VIP_SUCCESS;

  /* Create input/output buffers, prepare network */
  status = vnn_CreateInOutBufPrepareNetwork(network_items);
  _CHECK_STATUS(status, final);
  /* Set input/output buffers */
  status = vnn_SetNetworkInOut(network_items);
  _CHECK_STATUS(status, final);

final:
  return status;
}

vip_status_e vnn_RunNeuralNetwork(vip_network_items *network_items) {
  vip_status_e status = VIP_SUCCESS;
  vip_int32_t i = 0, loop = 1;
  char *loop_s;
  vip_uint64_t tmsStart, tmsEnd, sigStart, sigEnd;
  float msVal, usVal;

  loop_s = getenv("VNN_LOOP_TIME");
  if (loop_s) {
    loop = atoi(loop_s);
  }

  /* Run network */
  tmsStart = get_perf_count();
  printf("Start run graph [%d] times...\n", loop);
  for (i = 0; i < loop; i++) {
    sigStart = get_perf_count();
    status = vip_run_network(network_items->network);
    _CHECK_STATUS(status, final);
    sigEnd = get_perf_count();
    msVal = (float)(sigEnd - sigStart) / 1000000;
    usVal = (float)(sigEnd - sigStart) / 1000;
    printf("Run the %d time: %.2fms or %.2fus\n", (i + 1), msVal, usVal);
  }
  tmsEnd = get_perf_count();
  msVal = (float)(tmsEnd - tmsStart) / 1000000;
  usVal = (float)(tmsEnd - tmsStart) / 1000;
  printf("vip run network execution time:\n");
  printf("Total   %.2fms or %.2fus\n", msVal, usVal);
  printf("Average %.2fms or %.2fus\n", (float)msVal / loop,
         (float)usVal / loop);

final:
  return status;
}

vip_status_e vnn_PostProcessNeuralNetwork(vip_network_items *network_items) {
  return save_output_data(network_items);
}

vip_status_e vnn_ReleaseNeuralNetwork(vip_network_items *network_items) {
  vip_status_e status = VIP_SUCCESS;

  status = destroy_network(network_items);
  _CHECK_STATUS(status, final);
  destroy_network_items(network_items);
  status = vip_destroy();
  _CHECK_STATUS(status, final);

final:
  return status;
}

/*-------------------------------------------
                  Main Functions
-------------------------------------------*/
int main(int argc, char **argv) {
  vip_status_e status = VIP_SUCCESS;
  vip_network_items *network_items = VIP_NULL;

  if (argc < 3) {
    printf("%s\n", usage);
    printf("Arguments count %d is incorrect!\n", argc);
    return -1;
  }

  /* Initialize  vip lite */
  status = vip_init(7 * 1024 * 1024);
  _CHECK_STATUS(status, final);

  /* Initialize network items */
  status = vnn_InitNetworkItem(&network_items, argc, argv);
  _CHECK_STATUS(status, final);

  /* Create the neural network */
  status = vnn_CreateNeuralNetwork(network_items);
  _CHECK_STATUS(status, final);

  /* Pre process the input/output data */
  status = vnn_PreProcessNeuralNetwork(network_items);
  _CHECK_STATUS(status, final);

  /* Run the neural network */
  status = vnn_RunNeuralNetwork(network_items);
  _CHECK_STATUS(status, final);

  /* Post process output data */
  status = vnn_PostProcessNeuralNetwork(network_items);
  _CHECK_STATUS(status, final);

final:
  /* Destroy resources */
  status = vnn_ReleaseNeuralNetwork(network_items);
  return status;
}
