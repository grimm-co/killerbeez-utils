#pragma once

#include <Windows.h>
#include <stdint.h>

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the UTILS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// UTILS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef UTILS_EXPORTS
#define UTILS_API __declspec(dllexport)
#else
#define UTILS_API __declspec(dllimport)
#endif

UTILS_API int is_process_alive(HANDLE process);
UTILS_API int start_process_and_write_to_stdin(char * cmd_line, char * input, size_t input_length, HANDLE * process_out);
UTILS_API int start_process_and_write_to_stdin_and_save_pipes_timeout(char * cmd_line, char * input, size_t input_length, HANDLE * process_out, HANDLE * pipe_rd_ptr, HANDLE * pipe_wr_ptr, DWORD timeout_ms);
UTILS_API int WriteToPipe(HANDLE process, HANDLE pipe_wr, HANDLE pipe_rd, char * input, size_t input_length, DWORD timeout_ms);
UTILS_API int FlushPipe(HANDLE pipe_rd);
UTILS_API wchar_t * convert_char_array_to_wchar(char * string, wchar_t * out_buffer);
UTILS_API int write_buffer_to_file(char * filename, char * buffer, size_t length);
UTILS_API int read_file(char * filename, char **buffer);
UTILS_API char * get_temp_filename(char * suffix);
UTILS_API int file_exists(char * path);
UTILS_API void print_hex(char * data, size_t size);

//Argument parser helpers

#define IF_ARG_OPTION(x, y)           \
if(!strcmp(argv[i], x) && i+1 < argc) \
{                                     \
	y = argv[i + 1];                  \
	i++;                              \
}
#define IF_ARGINT_OPTION(x, y)        \
if(!strcmp(argv[i], x) && i+1 < argc) \
{                                     \
	y = atoi(argv[i + 1]);            \
	i++;                              \
}
#define IF_ARGDOUBLE_OPTION(x, y)     \
if(!strcmp(argv[i], x) && i+1 < argc) \
{                                     \
	y = atof(argv[i + 1]);            \
	i++;                              \
}
#define IF_ARG_SET_TRUE(x, y)         \
if(!strcmp(argv[i], x))               \
{                                     \
	y = 1;                            \
}

#define ELSE_IF_ARG_OPTION(x, y)       else IF_ARG_OPTION(x,y)
#define ELSE_IF_ARGINT_OPTION(x, y)    else IF_ARGINT_OPTION(x,y)
#define ELSE_IF_ARGDOUBLE_OPTION(x, y) else IF_ARGDOUBLE_OPTION(x,y)
#define ELSE_IF_ARG_SET_TRUE(x, y)     else IF_ARG_SET_TRUE(x,y)
