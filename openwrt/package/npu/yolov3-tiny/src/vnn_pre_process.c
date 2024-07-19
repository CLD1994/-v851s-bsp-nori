/****************************************************************************
 *   Generated by ACUITY 5.24.0
 *
 *   Neural Network appliction pre-process source file
 ****************************************************************************/
/*-------------------------------------------
                Includes
-------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image_utils.h"
#include "vnn_global.h"
#include "vnn_pre_process.h"

#define _BASETSD_H
/*-------------------------------------------
                  Functions
-------------------------------------------*/
unsigned int load_file(const char *name, void *dst) {
  FILE *fp = fopen(name, "rb");
  unsigned int size = 0;
  size_t read_count = 0;

  if (fp != NULL) {
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    read_count = fread(dst, size, 1, fp);
    if (read_count != 1) {
      printf("Read file %s failed.\n", name);
    }

    fclose(fp);
  } else {
    printf("Load file %s failed.\n", name);
  }

  return size;
}

unsigned int get_file_size(const char *name) {
  FILE *fp = fopen(name, "rb");
  unsigned int size = 0;

  if (fp != NULL) {
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    fclose(fp);
  } else {
    printf("Checking file %s failed.\n", name);
  }

  return size;
}

vip_status_e
vnn_CreateInOutBufPrepareNetwork(vip_network_items *network_items) {
  vip_status_e status = VIP_SUCCESS;
  int input_num = 0, i = 0;
  vip_buffer_create_params_t buf_param;

  /* Get network input num */
  status = vip_query_network(network_items->network,
                             VIP_NETWORK_PROP_INPUT_COUNT, &input_num);
  _CHECK_STATUS(status, final);
  if (input_num != network_items->input_count) {
    printf("Error: Graph need %d inputs, but enter %d inputs!!!\n", input_num,
           network_items->input_count);
    status = VIP_ERROR_MISSING_INPUT_OUTPUT;
    return status;
  }

  /* Create input buffers */
  network_items->input_buffers =
      (vip_buffer *)malloc(sizeof(vip_buffer) * input_num);
  for (i = 0; i < input_num; i++) {
    memset(&buf_param, 0, sizeof(buf_param));
    status =
        vip_query_input(network_items->network, i, VIP_BUFFER_PROP_DATA_FORMAT,
                        &buf_param.data_format);
    _CHECK_STATUS(status, final);
    status = vip_query_input(network_items->network, i,
                             VIP_BUFFER_PROP_NUM_OF_DIMENSION,
                             &buf_param.num_of_dims);
    _CHECK_STATUS(status, final);
    status =
        vip_query_input(network_items->network, i,
                        VIP_BUFFER_PROP_SIZES_OF_DIMENSION, buf_param.sizes);
    _CHECK_STATUS(status, final);
    status =
        vip_query_input(network_items->network, i, VIP_BUFFER_PROP_QUANT_FORMAT,
                        &buf_param.quant_format);
    _CHECK_STATUS(status, final);
    switch (buf_param.quant_format) {
    case VIP_BUFFER_QUANTIZE_DYNAMIC_FIXED_POINT:
      status = vip_query_input(network_items->network, i,
                               VIP_BUFFER_PROP_FIXED_POINT_POS,
                               &buf_param.quant_data.dfp.fixed_point_pos);
      _CHECK_STATUS(status, final);
      break;
    case VIP_BUFFER_QUANTIZE_TF_ASYMM:
      status =
          vip_query_input(network_items->network, i, VIP_BUFFER_PROP_TF_SCALE,
                          &buf_param.quant_data.affine.scale);
      _CHECK_STATUS(status, final);
      status = vip_query_input(network_items->network, i,
                               VIP_BUFFER_PROP_TF_ZERO_POINT,
                               &buf_param.quant_data.affine.zeroPoint);
      _CHECK_STATUS(status, final);
      break;
    case VIP_BUFFER_QUANTIZE_NONE:
    default:
      break;
    }

    status = vip_create_buffer(&buf_param, sizeof(buf_param),
                               &network_items->input_buffers[i]);
    _CHECK_STATUS(status, final);
  }

  /* Create output buffers */
  status =
      vip_query_network(network_items->network, VIP_NETWORK_PROP_OUTPUT_COUNT,
                        &network_items->output_count);
  network_items->output_buffers =
      (vip_buffer *)malloc(sizeof(vip_buffer) * network_items->output_count);
  for (i = 0; i < network_items->output_count; i++) {
    memset(&buf_param, 0, sizeof(buf_param));
    status =
        vip_query_output(network_items->network, i, VIP_BUFFER_PROP_DATA_FORMAT,
                         &buf_param.data_format);
    _CHECK_STATUS(status, final);
    status = vip_query_output(network_items->network, i,
                              VIP_BUFFER_PROP_NUM_OF_DIMENSION,
                              &buf_param.num_of_dims);
    _CHECK_STATUS(status, final);
    status =
        vip_query_output(network_items->network, i,
                         VIP_BUFFER_PROP_SIZES_OF_DIMENSION, buf_param.sizes);
    _CHECK_STATUS(status, final);
    status =
        vip_query_output(network_items->network, i,
                         VIP_BUFFER_PROP_QUANT_FORMAT, &buf_param.quant_format);
    _CHECK_STATUS(status, final);
    switch (buf_param.quant_format) {
    case VIP_BUFFER_QUANTIZE_DYNAMIC_FIXED_POINT:
      status = vip_query_output(network_items->network, i,
                                VIP_BUFFER_PROP_FIXED_POINT_POS,
                                &buf_param.quant_data.dfp.fixed_point_pos);
      _CHECK_STATUS(status, final);
      break;
    case VIP_BUFFER_QUANTIZE_TF_ASYMM:
      status =
          vip_query_output(network_items->network, i, VIP_BUFFER_PROP_TF_SCALE,
                           &buf_param.quant_data.affine.scale);
      _CHECK_STATUS(status, final);
      status = vip_query_output(network_items->network, i,
                                VIP_BUFFER_PROP_TF_ZERO_POINT,
                                &buf_param.quant_data.affine.zeroPoint);
      _CHECK_STATUS(status, final);
      break;
    case VIP_BUFFER_QUANTIZE_NONE:
    default:
      break;
    }
    status = vip_create_buffer(&buf_param, sizeof(buf_param),
                               &network_items->output_buffers[i]);
    _CHECK_STATUS(status, final);
  }

  /* Prepare network */
  status = vip_prepare_network(network_items->network);
  _CHECK_STATUS(status, final);

final:
  return status;
}

vip_status_e vnn_SetNetworkInOut(vip_network_items *network_items) {
  vip_status_e status = VIP_SUCCESS;
  void *data = NULL;
  char *file_name = NULL;
  vip_uint32_t file_size = 0;
  vip_uint32_t buff_size = 0;
  int i;

  /* Load input buffer data */
  for (i = 0; i < network_items->input_count; i++) {
    file_name = network_items->input_names[i];
    data = vip_map_buffer(network_items->input_buffers[i]);
    buff_size = vip_get_buffer_size(network_items->input_buffers[i]);
    // file_size = load_file((const char *)file_name, data);
    file_size = decode_jpeg((const char *)file_name, data);
    if (buff_size != file_size) {
      printf(
          "Error: Input size mismatch for %s, file data size:%u, expected:%u\n",
          file_name, file_size, buff_size);
      status = VIP_ERROR_INVALID_ARGUMENTS;
      goto final;
    }
    /* Set input */
    status = vip_set_input(network_items->network, i,
                           network_items->input_buffers[i]);
    _CHECK_STATUS(status, final);
  }

  /* Set output */
  for (i = 0; i < network_items->output_count; i++) {
    status = vip_set_output(network_items->network, i,
                            network_items->output_buffers[i]);
    _CHECK_STATUS(status, final);
  }

final:
  return status;
}
