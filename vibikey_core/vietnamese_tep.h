/*
 * VibiKey - Vietnamese Input Method
 * Shared TEIP/VNI implementation header
 */

#ifndef VIBIKEY_VIETNAMESE_TEP_H
#define VIBIKEY_VIETNAMESE_TEP_H

#ifdef __cplusplus
extern "C" {
#endif

#define VIBIKEY_METHOD_RAW      0
#define VIBIKEY_METHOD_TEIP     1
#define VIBIKEY_METHOD_VNI      2

#define VIBIKEY_MAX_KEY_SEQUENCE   10
#define VIBIKEY_MAX_OUTPUT_LENGTH  32

int vibikey_process_input(const char *sequence, char *output, int max_output_len, int method);
int vibikey_convert_word(const char *word, char *output, int max_len, int method);
int vibikey_modifier_pressed(int vk_code);

#ifdef __cplusplus
}
#endif

#endif