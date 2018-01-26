#pragma once

#include "jansson.h"

#ifdef __cplusplus
extern "C" {
#endif

// Some macros to make parsing options easier

#define PARSE_OPTION_INT(state, options, name, name_literal, fail_func)        \
	int result_##name = 0;                                                     \
	int tempi_##name = get_int_options(options, name_literal, &result_##name); \
	if (result_##name < 0)                                                     \
	{                                                                          \
		fail_func(state);                                                      \
		return NULL;                                                           \
	}                                                                          \
	else if (result_##name > 0)                                                \
	{                                                                          \
		state->name = tempi_##name;                                            \
	}

#define PARSE_OPTION_UINT64T(state, options, name, name_literal, fail_func)             \
	int result_##name = 0;                                                              \
	uint64_t tempi_##name = get_uint64t_options(options, name_literal, &result_##name); \
	if (result_##name < 0)                                                              \
	{                                                                                   \
		fail_func(state);                                                               \
		return NULL;                                                                    \
	}                                                                                   \
	else if (result_##name > 0)                                                         \
	{                                                                                   \
		state->name = tempi_##name;                                                     \
	}

/**
 * The samae as PARSE_OPTION_UINT64T, but allows you to specify the name to use for the temporary variables
 */
#define PARSE_OPTION_UINT64T_TEMP(state, options, name, name_literal, fail_func, temp_name) \
	int result_##temp_name = 0;                                                                   \
	uint64_t tempi_##temp_name = get_uint64t_options(options, name_literal, &result_##temp_name); \
	if (result_##temp_name < 0)                                                                   \
	{                                                                                             \
		fail_func(state);                                                                         \
		return NULL;                                                                              \
	}                                                                                             \
	else if (result_##temp_name > 0)                                                              \
	{                                                                                             \
		state->name = tempi_##temp_name;                                                          \
	}

#define PARSE_OPTION_STRING(state, options, name, name_literal, fail_func)           \
	int result_##name = 0;                                                           \
	char * temps_##name = get_string_options(options, name_literal, &result_##name); \
	if (result_##name < 0)                                                           \
	{                                                                                \
		fail_func(state);                                                            \
		return NULL;                                                                 \
	}                                                                                \
	else if (result_##name > 0)                                                      \
	{                                                                                \
		if(state->name)                                                              \
			free(state->name);                                                       \
		state->name = temps_##name;                                                  \
	}

#define PARSE_OPTION_ARRAY(state, options, name, count, name_literal, fail_func)                     \
	int result_##name = 0;                                                                           \
	size_t count_##name = 0;                                                                            \
	char ** temps_##name = get_array_options(options, name_literal, &count_##name, &result_##name);  \
	if (result_##name < 0)                                                                           \
	{                                                                                                \
		fail_func(state);                                                                            \
		return NULL;                                                                                 \
	}                                                                                                \
	else if (result_##name > 0)                                                                      \
	{                                                                                                \
		if(state->name)                                                                              \
			free(state->name);                                                                       \
		state->name = temps_##name;                                                                  \
		state->count = count_##name;                                                                 \
	}                            

// Some macros to make iterating json arrays easier

#define FOREACH_OBJECT_JSON_ARRAY_ITEM_BEGIN(state, name, name_str, item, result)                    \
    do {                                                                                             \
		json_t * root##name, *option_array##name;                                                    \
		size_t i##name;                                                                              \
																									 \
		result = -1;                                                                                 \
		root##name = get_root_option_json_object(state);                                             \
		if (root##name)                                                                              \
		{                                                                                            \
			option_array##name = json_object_get(root##name, name_str);                              \
			if (!option_array##name || !json_is_array(option_array##name))                           \
				json_decref(root##name);                                                             \
			else                                                                                     \
			{                                                                                        \
				result = 1;                                                                          \
				for (i##name = 0; i##name < json_array_size(option_array##name); i##name++)          \
				{                                                                                    \
					item = json_array_get(option_array##name, i##name);

#define FOREACH_OBJECT_JSON_ARRAY_ITEM_END(name)                                                     \
				}                                                                                    \
			}                                                                                        \
			json_decref(root##name);                                                                 \
		}                                                                                            \
	} while (0);


// Some macros to make generating objects easier

#define ADD_ITEM1(temp, arg1, dest, func, name)       \
	temp = func(arg1);                                \
    if(!temp) return NULL;                            \
	json_object_set_new(dest, name, temp);

#define ADD_ITEM2(temp, arg1, arg2, dest, func, name) \
	temp = func(arg1, arg2);                          \
    if(!temp) return NULL;                            \
	json_object_set_new(dest, name, temp);


#define ADD_STRING(temp, arg1, dest, name)        ADD_ITEM1(temp, arg1, dest, json_string, name)
#define ADD_INT(temp, arg1, dest, name)           ADD_ITEM1(temp, arg1, dest, json_integer, name)
#define ADD_MEM(temp, arg1, arg2, dest, name)     ADD_ITEM2(temp, arg1, arg2, dest, json_mem, name)


JANSSON_API char * get_string_options(char * options, char * option_name, int * result);
JANSSON_API char * get_string_options_from_json(json_t * root, char * option_name, int * result);

JANSSON_API char * get_mem_options(char * options, char * option_name, int * result);
JANSSON_API char * get_mem_options_from_json(json_t * root, char * option_name, int * result);

JANSSON_API int get_int_options(char * options, char * option_name, int * result);
JANSSON_API int get_int_options_from_json(json_t * root, char * option_name, int * result);

JANSSON_API uint64_t get_uint64t_options(char * options, char * option_name, int * result);
JANSSON_API uint64_t get_uint64t_options_from_json(json_t * root, char * option_name, int * result);

JANSSON_API char ** get_array_options(char * options, char * option_name, size_t * count, int * result);

JANSSON_API json_t * get_root_option_json_object(char * options);

JANSSON_API char * add_string_option_to_json(char * root_options, char * new_option_name, char * new_value);
JANSSON_API char * add_int_option_to_json(char * root_options, char * new_option_name, int new_value);

#ifdef __cplusplus
}
#endif
