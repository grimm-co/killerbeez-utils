#include "jansson.h"
#include "jansson_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * This function parses the given JSON string and converts it into a json_t object
 * @param json_string - a JSON character buffer that should be converted into a json_t
 * @return - a json_t object on success, or NULL on failure
 */
json_t * get_root_option_json_object(char * json_string)
{
	json_t * root;
	json_error_t error;

	root = json_loads(json_string, 0, &error);
	if (!root)
	{
		fprintf(stderr, "json error in options: on line %d: %s\n", error.line, error.text);
		return NULL;
	}

	if (!json_is_object(root))
	{
		fprintf(stderr, "json error in options: root is not an array\n");
		json_decref(root);
		return NULL;
	}

	return root;
}

/**
 * Gets a string attribute value from a json_t object
 * @param root - the json_t object to get the attribute from
 * @param option_name - the name of the attribute to get
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found, -1
 * if the attribute is found but not a string, and 1 if the attribute is found and
 * a string.
 * @param should_free - whether or not to free the root json_t parameter after finding
 * the attribute
 * @return - the string value of the specified attribute on success, or NULL if the attribute
 * wasn't found or the wrong type. The return value should be freed by the caller.
 */
static char * get_string_options_inner(json_t * root, char * option_name, int * result, int should_free)
{
	json_t *option_item;
	char * option_value;

	option_item = json_object_get(root, option_name);
	if (!option_item)
	{
		if (should_free)
			json_decref(root);
		*result = 0;
		return NULL;
	}

	if (!json_is_string(option_item))
	{
		*result = -1;
		fprintf(stderr, "error: option item %s is expected to be a string\n", option_name);
		if (should_free)
			json_decref(root);
		return NULL;
	}

	option_value = strdup(json_string_value(option_item));
	*result = 1;
	if (should_free)
		json_decref(root);
	return option_value;
}

/**
 * Gets a string attribute value from a JSON string
 * @param json_string - the json_t string to parse and obtain the attribute value from
 * @param option_name - the name of the attribute to get
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found, -1
 * if the attribute is found but not a string, and 1 if the attribute is found and
 * a string.
 * @return - the string value of the specified attribute on success, or NULL if the json
 * couldn't be parsed, the attribute wasn't found, or the attribute was the wrong type.
 * The return value should be freed by the caller.
 */
char * get_string_options(char * json_string, char * option_name, int * result)
{
	json_t * root;

	*result = -1;
	root = get_root_option_json_object(json_string);
	if (!root)
		return NULL;
	return get_string_options_inner(root, option_name, result, 1);
}


/**
 * Gets a string attribute value from a JSON string
 * @param root - the json_t object to get the attribute from
 * @param option_name - the name of the attribute to get
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found, -1
 * if the attribute is found but not a string, and 1 if the attribute is found and
 * a string.
 * @return - the string value of the specified attribute on success, or NULL if the
 * attribute wasn't found or the wrong type. The return value should be freed by the caller.
 */
char * get_string_options_from_json(json_t * root, char * option_name, int * result)
{
	return get_string_options_inner(root, option_name, result, 0);
}

/**
 * Gets a mem attribute value from a json_t object
 * @param root - the json_t object to get the attribute from
 * @param option_name - the name of the attribute to get
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found, -1
 * if the attribute is found but not a mem, and 1 if the attribute is found and
 * a mem.
 * @param should_free - whether or not to free the root json_t parameter after finding
 * the attribute
 * @return - the mem value of the specified attribute on success, or NULL if the attribute
 * wasn't found or the wrong type.  The return value should be freed by the caller.
 */
static char * get_mem_options_inner(json_t * root, char * option_name, int * result, int should_free)
{
	json_t *option_item;
	char * option_value;
	size_t length;

	option_item = json_object_get(root, option_name);
	if (!option_item)
	{
		if (should_free)
			json_decref(root);
		*result = 0;
		return NULL;
	}

	if (!json_is_mem(option_item))
	{
		*result = -1;
		fprintf(stderr, "error: option item %s is expected to be a mem\n", option_name);
		if (should_free)
			json_decref(root);
		return NULL;
	}

	length = json_mem_length(option_item);
	option_value = malloc(length);
	memcpy(option_value, json_mem_value(option_item), length);
	*result = 1;
	if (should_free)
		json_decref(root);
	return option_value;
}

/**
 * Gets a mem attribute value from a JSON string
 * @param json_string - the json_t string to parse and obtain the attribute value from
 * @param option_name - the name of the attribute to get
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found, -1
 * if the attribute is found but not a mem, and 1 if the attribute is found and
 * a mem.
 * @return - the mem value of the specified attribute on success, or NULL if the json
 * couldn't be parsed, the attribute wasn't found, or the attribute was the wrong type.
 * The return value should be freed by the caller.
 */
char * get_mem_options(char * json_string, char * option_name, int * result)
{
	json_t * root;

	*result = -1;
	root = get_root_option_json_object(json_string);
	if (!root)
		return NULL;
	return get_mem_options_inner(root, option_name, result, 1);
}

/**
 * Gets a mem attribute value from a json_t object
 * @param root - the json_t object to get the attribute from
 * @param option_name - the name of the attribute to get
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found, -1
 * if the attribute is found but not a mem, and 1 if the attribute is found and
 * a mem.
 * @return - the mem value of the specified attribute on success, or NULL if the attribute
 * wasn't found or the wrong type. The return value should be freed by the caller.
 */
char * get_mem_options_from_json(json_t * root, char * option_name, int * result)
{
	return get_mem_options_inner(root, option_name, result, 0);
}

/**
 * Gets an integer attribute value from a json_t object
 * @param root - the json_t object to get the attribute from
 * @param option_name - the name of the attribute to get
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found, -1
 * if the attribute is found but not an integer, and 1 if the attribute is found and
 * an integer.
 * @param should_free - whether or not to free the root json_t parameter after finding
 * the attribute
 * @return - the integer value of the specified attribute on success, or -1 if the attribute
 * wasn't found or the wrong type.  Check the value of the result parameter to differentiate
 * between the attribute value of -1 or failure to get the value.
 */
static long long get_int_options_inner(json_t * root, char * option_name, int * result, int should_free)
{
	json_t *option_item;
	long long option_value;

	option_item = json_object_get(root, option_name);
	if (!option_item)
	{
		if (should_free)
			json_decref(root);
		*result = 0;
		return -1;
	}

	if (!json_is_integer(option_item))
	{
		*result = -1;
		fprintf(stderr, "error: option item %s is expected to be an integer\n", option_name);
		if (should_free)
			json_decref(root);
		return -1;
	}

	option_value = json_integer_value(option_item);
	*result = 1;
	if (should_free)
		json_decref(root);
	return option_value;
}

/**
 * Gets an integer attribute value from a JSON string
 * @param json_string - the json_t string to parse and obtain the attribute value from
 * @param option_name - the name of the attribute to get
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found, -1
 * if the attribute is found but not an integer, and 1 if the attribute is found and
 * an integer.
 * @return - the integer value of the specified attribute on success, or -1 if the json
 * couldn't be parsed, the attribute wasn't found, or the attribute was the wrong type.
 * Check the value of the result parameter to differentiate between the attribute value
 * of -1 or failure to get the value.s
 */
int get_int_options(char * json_string, char * option_name, int * result)
{
	json_t * root;

	*result = -1;
	root = get_root_option_json_object(json_string);
	if (!root)
		return -1;
	return (int)get_int_options_inner(root, option_name, result, 1);
}

/**
* Gets an integer attribute value from a json_t object
* @param root - the json_t object to get the attribute from
* @param option_name - the name of the attribute to get
* @param result - a pointer to an integer to return the results of trying to get
* the attribute value.  The value 0 is returned if the attribute isn't found, -1
* if the attribute is found but not an integer, and 1 if the attribute is found and
* an integer.
* @return - the integer value of the specified attribute on success, or -1 if the attribute
* wasn't found or the wrong type.  Check the value of the result parameter to differentiate
* between the attribute value of -1 or failure to get the value.
*/
int get_int_options_from_json(json_t * root, char * option_name, int * result)
{
	return (int)get_int_options_inner(root, option_name, result, 0);
}

/**
* Gets a uint64_t attribute value from a JSON string
* @param json_string - the json_t string to parse and obtain the attribute value from
* @param option_name - the name of the attribute to get
* @param result - a pointer to an integer to return the results of trying to get
* the attribute value.  The value 0 is returned if the attribute isn't found, -1
* if the attribute is found but not an integer, and 1 if the attribute is found and
* an integer.
* @return - the uint64_t value of the specified attribute on success, or -1 if the json
* couldn't be parsed, the attribute wasn't found, or the attribute was the wrong type.
* Check the value of the result parameter to differentiate between the attribute value
* of -1 or failure to get the value.s
*/
uint64_t get_uint64t_options(char * json_string, char * option_name, int * result)
{
	json_t * root;

	*result = -1;
	root = get_root_option_json_object(json_string);
	if (!root)
		return -1;
	return (uint64_t)get_int_options_inner(root, option_name, result, 1);
}

/**
* Gets a uint64_t attribute value from a json_t object
* @param root - the json_t object to get the attribute from
* @param option_name - the name of the attribute to get
* @param result - a pointer to an integer to return the results of trying to get
* the attribute value.  The value 0 is returned if the attribute isn't found, -1
* if the attribute is found but not an integer, and 1 if the attribute is found and
* an integer.
* @return - the uint64_t value of the specified attribute on success, or -1 if the attribute
* wasn't found or the wrong type.  Check the value of the result parameter to differentiate
* between the attribute value of -1 or failure to get the value.
*/
uint64_t get_uint64t_options_from_json(json_t * root, char * option_name, int * result)
{
	return (uint64_t)get_int_options_inner(root, option_name, result, 0);
}

/**
 * Gets a string array attribute value from a JSON string
 * @param json_string - the json_t string to parse and obtain the attribute value from
 * @param option_name - the name of the attribute to get
 * @param count - a pointer to a size_t object used to return the number of items found
 * in the array attribute value
 * @param result - a pointer to an integer to return the results of trying to get
 * the attribute value.  The value 0 is returned if the attribute isn't found; -1
 * if the attribute is found but not an array, contains elements that aren't strings, or
 * on allocation failures; and 1 if the attribute is found and a string array.
 * @return - the string array value of the specified attribute on success, or NULL if the
 * attribute wasn't found or the wrong type.  The returned array and all of its elements should
 * be freed by the caller.
 */
char ** get_array_options(char * json_string, char * option_name, size_t * count, int * result)
{
	json_t * root, *option_array, *option_item;
	char ** option_strings;
	size_t i;

	*result = -1;
	root = get_root_option_json_object(json_string);
	if (!root)
		return NULL;

	option_array = json_object_get(root, option_name);
	if (!option_array)
	{
		json_decref(root);
		*result = 0;
		return NULL;
	}

	if (!json_is_array(option_array))
	{
		fprintf(stderr, "error: option item %s is expected to be a array\n", option_name);
		json_decref(root);
		return NULL;
	}

	*count = json_array_size(option_array);
	option_strings = malloc(*count * sizeof(char *));
	if (!option_strings)
	{
		fprintf(stderr, "error: couldn't allocate array for option %s (%zu items)\n", option_name, *count);
		json_decref(root);
		return NULL;
	}
	for (i = 0; i < *count; i++)
	{
		option_item = json_array_get(option_array, i);
		if (!json_is_string(option_item))
		{
			fprintf(stderr, "error: option %zu in array %s is expected to be a string\n", i, option_name);
			json_decref(root);
			free(option_strings);
			return NULL;
		}
		option_strings[i] = strdup(json_string_value(option_item));
	}

	*result = 1;
	json_decref(root);
	return option_strings;
}

/**
 * Adds a new attribute to an existing json string.
 * @param root_options - the json_t string to parse and add an attribute to
 * @param new_option_name - the name of the attribute to create
 * @param new_value_string - If specified, the new attribute has the type string
 * and is given the value of this parameter.
 * @param new_value_int - If new_value_string is NULL, the new attribute has the
 * type int and the value specified in this parameter.
 * @return - NULL on error, or the json string passed in the root_options parameter
 * with the added attribute as requested
 */
static char * add_option_to_json(char * root_options, char * new_option_name, char * new_value_string, int new_value_int)
{
	json_t * root, *temp;
	char * ret;

	root = get_root_option_json_object(root_options);
	if (!root)
		return NULL;

	//Add the new item
	if (new_value_string) {
		ADD_STRING(temp, new_value_string, root, new_option_name);
	} else {
		ADD_INT(temp, new_value_int, root, new_option_name);
	}

	ret = json_dumps(root, 0);
	json_decref(root);
	return ret;
}

/**
 * Adds a new string attribute to an existing json string.
 * @param root_options - the json_t string to parse and add an attribute to
 * @param new_option_name - the name of the attribute to create
 * @param new_value - the new attribute's value
 * @return - NULL on error, or the json string passed in the root_options parameter
 * with the added attribute as requested
 */
char * add_string_option_to_json(char * root_options, char * new_option_name, char * new_value)
{
	if (!new_value)
		return NULL;
	return add_option_to_json(root_options, new_option_name, new_value, 0);
}

/**
* Adds a new integer attribute to an existing json string.
* @param root_options - the json_t string to parse and add an attribute to
* @param new_option_name - the name of the attribute to create
* @param new_value - the new attribute's value
* @return - NULL on error, or the json string passed in the root_options parameter
* with the added attribute as requested
*/
char * add_int_option_to_json(char * root_options, char * new_option_name, int new_value)
{
	return add_option_to_json(root_options, new_option_name, NULL, new_value);
}
