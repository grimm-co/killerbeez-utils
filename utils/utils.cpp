#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Windows API
#include <windows.h> 
#include <process.h>
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

static int CreateChildProcess(TCHAR * cmd_line, HANDLE read_pipe, HANDLE * process_out);

/**
* This function converts a char * to a wchar *
* @param - The char * string that should be converted to a wchar * string
* @param - A wchar * buffer that the converted string should be placed into.  If NULL,
* this function will allocate a wchar * buffer to place the converted string into.
* @return - A pointer to the converted string
*/
wchar_t * convert_char_array_to_wchar(char * string, wchar_t * out_buffer)
{
	size_t size = (strlen(string) + 1) * sizeof(wchar_t);
	size_t converted_length = 0;

	if (!out_buffer)
	{
		out_buffer = (wchar_t *)malloc(size);
		if (!out_buffer)
			return NULL;
	}

	mbstowcs_s(&converted_length, out_buffer, strlen(string) + 1, string, size);
	return out_buffer;
}


#define CLOSE_PIPES() \
	if(pipe_rd) CloseHandle(pipe_rd); \
	if(pipe_wr) CloseHandle(pipe_wr);

#define MAX_CMD_LEN 10*4096
#define MAX_STANDARD_IN_PIPE_SIZE 8*1024 *1024 //8MB

/**
 * This function starts a process and writes to the stdin of the process.
 * @param cmd_line - The command line of the new process to start
 * @param input - a buffer that should be pasesd to the newly created process's stdin
 * @param input_length - The length of the input parameter
 * @param process_out - a pointer to a HANDLE that will be filled in with a handle to the newly created process
 * @param pipe_rd_ptr - a pointer to a HANDLE that will be filled in with the read end of the stdin pipe for the new process.
 * If pipe_rd_ptr is NULL, the read end of the stdin pipe will be closed instead.
 * @param pipe_wr_ptr - a pointer to a HANDLE that will be filled in with the write end of the stdin pipe for the new process.
 * If pipe_wr_ptr is NULL, the write end of the stdin pipe will be closed instead.
 * @param timeout_ms - The maximum number of milliseconds to wait when writing to the newly created process's stdin pipe.
 * @return - zero on success, non-zero on failure
 */
UTILS_API int start_process_and_write_to_stdin_and_save_pipes_timeout(char * cmd_line, char * input, size_t input_length, HANDLE * process_out, HANDLE * pipe_rd_ptr, HANDLE * pipe_wr_ptr, DWORD timeout_ms)
{
	SECURITY_ATTRIBUTES saAttr;
	TCHAR converted_cmd_line[MAX_CMD_LEN];
	size_t converted_cmd_line_length = 0;
	int ret;
	HANDLE pipe_rd, pipe_wr;

	//Mark the process as not started in case we error out
	*process_out = NULL;
	if (strlen(cmd_line) > MAX_CMD_LEN)
		return 1;

	//Convert the command line
	if (!convert_char_array_to_wchar(cmd_line, converted_cmd_line))
		return 1;

	// Set the bInheritHandle flag so pipe handles are inherited. 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDIN. 
	if (!CreatePipe(&pipe_rd, &pipe_wr, &saAttr, min(input_length, MAX_STANDARD_IN_PIPE_SIZE)))
		return 1;

	// Ensure the write handle to the pipe for STDIN is not inherited. 
	if (!SetHandleInformation(pipe_wr, HANDLE_FLAG_INHERIT, 0))
	{
		CLOSE_PIPES();
		return 1;
	}

	// Create the child process. 
	if (CreateChildProcess(converted_cmd_line, pipe_rd, process_out))
	{
		CLOSE_PIPES();
		return 1;
	}

	//Write the input buffer
	ret = 0;
	if (input && input_length > 0)
	{
		if (WriteToPipe(*process_out, pipe_wr, pipe_rd, input, input_length, timeout_ms))
			ret = 1;
	}

	//Either save the pipes, or close them so we don't leak resources
	if (pipe_rd_ptr)
		*pipe_rd_ptr = pipe_rd;
	else
		CloseHandle(pipe_rd);
	if (pipe_wr_ptr)
		*pipe_wr_ptr = pipe_wr;
	else
		CloseHandle(pipe_wr);
	return ret;
}

/**
* This function starts a process and writes to the stdin of the process.
* @param cmd_line - The command line of the new process to start
* @param input - a buffer that should be pasesd to the newly created process's stdin
* @param input_length - The length of the input parameter
* @param process_out - a pointer to a HANDLE that will be filled in with a handle to the newly created process
* @return - zero on success, non-zero on failure
*/
UTILS_API int start_process_and_write_to_stdin(char * cmd_line, char * input, size_t input_length, HANDLE * process_out)
{
	return start_process_and_write_to_stdin_and_save_pipes_timeout(cmd_line, input, input_length, process_out, NULL, NULL, 0);
}

/**
 * This function starts a new process
 * @param cmd_line - The command line for the process to create
 * @param read_pipe - A handle to the read end of a pipe that should be assigned to the newly created process's stdin
 * @param process_out - A pointer toa HANDLE that will be filled in with a handle to the newly created process
 * @return - zero on success, non-zero on failure
 */
static int CreateChildProcess(TCHAR * cmd_line, HANDLE read_pipe, HANDLE * process_out)
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));


	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdInput = read_pipe;
	siStartInfo.hStdError = NULL;
	siStartInfo.hStdOutput = NULL;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	siStartInfo.wShowWindow = 1;

	// Create the child process. 
	bSuccess = CreateProcess(NULL,
		cmd_line,       // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

					   // If an error occurs, exit the application. 
	if (!bSuccess)
		return 1;

	CloseHandle(piProcInfo.hThread); //We don't need the thread handle
	*process_out = piProcInfo.hProcess;
	return 0;
}

#define MAX_WRITE_SIZE 8*1024*1024 //8MB

#define GET_FILETIME_DIFF_IN_MILLISECONDS(x,y,z) \
	ULARGE_INTEGER temp1##x##y, temp2##x##y; \
	temp1##x##y.LowPart = x.dwLowDateTime; temp1##x##y.HighPart = x.dwHighDateTime; \
	temp2##x##y.LowPart = y.dwLowDateTime; temp2##x##y.HighPart = y.dwHighDateTime; \
	z = (temp1##x##y.QuadPart - temp2##x##y.QuadPart) / 10000;

/**
 * Writes the given input buffer the a pipe, checking to make sure the other process hasn't died
 * and that there is room on the pipe to write.
 * @param process - The process that holds the read end of the pipe.
 * @param pipe_wr - The write end of the pipe that will be written to.
 * @param pipe_rd - The read end of the pipe being written to
 * @param input - a buffer to write to the the pipe_wr parameter
 * @param input_length - The length of the input parameter
 * @param timeout_ms - The maximum number of milliseconds to wait when writing to the pipe
 * @return - 0 on success (all bytes written to the pipe), 1 on failure
 */
UTILS_API int WriteToPipe(HANDLE process, HANDLE pipe_wr, HANDLE pipe_rd, char * input, size_t input_length, DWORD timeout_ms)
{
	DWORD dwWritten, out_size, total_in_pipe, timediff;
	size_t total_written = 0, write_size;
	BOOL bSuccess = FALSE;
	FILETIME start_time, time;

	GetSystemTimeAsFileTime(&start_time);
	while (total_written < input_length && is_process_alive(process))
	{
		if (!GetNamedPipeInfo(pipe_wr, NULL, &out_size, NULL, NULL))
			break;
		if (!PeekNamedPipe(pipe_rd, NULL, 0, NULL, &total_in_pipe, NULL))
			break;
		write_size = min(min(input_length - total_written, MAX_WRITE_SIZE), out_size - total_in_pipe);
		if (write_size == 0) //There's no room to write to the pipe
		{
			GetSystemTimeAsFileTime(&time);
			GET_FILETIME_DIFF_IN_MILLISECONDS(time, start_time, timediff);
			if (timeout_ms && timediff > timeout_ms)
				return 1;
			dwWritten = WaitForSingleObject(pipe_wr, timeout_ms);
		}
		else
		{
			bSuccess = WriteFile(pipe_wr, input + total_written, write_size, &dwWritten, NULL);
			if (!bSuccess) break;
			total_written += dwWritten;
		}
	}
	return total_written != input_length;
}

/**
 * Determines whether a file exists or not
 * @param path - The path of the file to check for existence
 * @return - 1 if the file exists, 0 otherwise
 */
UTILS_API int file_exists(char * path)
{
	WIN32_FIND_DATA FindFileData;
	wchar_t wide_path[MAX_PATH];
	convert_char_array_to_wchar(path, wide_path);
	HANDLE handle = FindFirstFile(wide_path, &FindFileData);
	int found = handle != INVALID_HANDLE_VALUE;
	if (found)
		FindClose(handle);
	return found;
}

/**
 * Flushes any input waiting on the given pipe
 * @param pipe_rd - a handle to the pipe that should be flushed
 * @return - 0 on success, non-zero on failure
 */
UTILS_API int FlushPipe(HANDLE pipe_rd)
{
	DWORD total_in_pipe, num_read;
	int failed;
	char * temp;

	if (!PeekNamedPipe(pipe_rd, NULL, 0, NULL, &total_in_pipe, NULL))
		return 1;
	if (!total_in_pipe)
		return 0;

	temp = (char *)malloc(total_in_pipe);
	failed = ReadFile(pipe_rd, temp, total_in_pipe, &num_read, NULL) != TRUE;
	free(temp);
	if (num_read != total_in_pipe)
		failed = 1;
	return failed;
}

/**
 * This function checks if a process is still alive
 * @param - a HANDLE to the process to check
 * @return - 1 if the process is alive, 0 if it is not, -1 on failure
 */
UTILS_API int is_process_alive(HANDLE process)
{
	DWORD exitCode;
	if (GetExitCodeProcess(process, &exitCode) == 0)
		return -1;
	return exitCode == STILL_ACTIVE;
}

/**
 * This function writes a buffer to the specified file.
 * @param filename - The filename to write the buffer to
 * @param buffer - The buffer to write
 * @param length - THe length of the buffer parameter
 * @param return - 0 on success, non-zero otherwise
 */
UTILS_API int write_buffer_to_file(char * filename, char * buffer, size_t length)
{
	int num_written;
	size_t total = 0;
	FILE * fp = NULL;
	int error = EACCES;

	//We need to do this in a loop, since we may need to wait for
	//a process to stop holding this file
	while (!fp && error == EACCES)
	{
		fp = fopen(filename, "wb+");
		error = errno;
	}
	if (!fp)
		return -1;

	while (total < length)
	{
		num_written = fwrite(buffer + total, 1, length - total, fp);
		if (num_written < 0 && errno != EAGAIN && errno != EINTR)
			break;
		else if (num_written > 0)
			total += num_written;
	}
	fclose(fp);
	return total != length;
}

/**
 * Reads a file from disk
 * @param filename - The filename of the file to read
 * @param buffer - A pointer to a character buffer that will be assigned a newly allocated
 * buffer to hold the file contents.  The caller should free this buffer.
 * @return - -1 on failure, otherwise the number of bytes read from the file
 */
int read_file(char * filename, char **buffer)
{
	FILE *fp;
	long fsize, total = 0, num_read;

	*buffer = NULL;

	fp = fopen(filename, "rb");
	if (!fp)
		return -1;

	//Get the size
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	*buffer = (char *)malloc(fsize + 1);
	if (!*buffer)
	{
		fclose(fp);
		return -1;
	}

	(*buffer)[fsize] = 0; //NULL terminate in case the caller wants to use it as a string
	while (total < fsize)
	{
		num_read = fread(*buffer + total, 1, fsize, fp);
		total += num_read;
	}

	fclose(fp);

	return fsize;
}

/**
 * Generates a temporary filename
 * @param suffix - Optionally, a suffix to append to the generated temporary filename.  If NULL,
 * no file extension will be added.
 * @return - NULL on failure, or a newly allocated character buffer holding the temporary filename.
 * The caller should free the returned buffer
 */
char * get_temp_filename(char * suffix)
{
	TCHAR temp_dir[MAX_PATH];
	TCHAR temp_filename[MAX_PATH];
	char * ret;
	size_t suffix_length = 0;

	//Get the temp filename
	if (GetTempPath(MAX_PATH, temp_dir) == 0)
		return NULL;
	GetTempFileName(temp_dir, L"fuzzfile", 0, temp_filename);

	//Add the suffix and convert it to a useable format
	if (suffix)
		suffix_length = strlen(suffix);
	ret = (char *)malloc(MAX_PATH + suffix_length);
	if (!ret)
		return NULL;

	memset(ret, 0, MAX_PATH + suffix_length);
	wcstombs(ret, temp_filename, MAX_PATH);
	unlink(ret); //Cleanup the file without the extension that GetTempFileName generated
	if(suffix)
		strncat(ret, suffix, MAX_PATH + suffix_length);

	return ret;
}

/**
* This function prints a data buffer in hex
*
* @param data - a char * data buffer
* @param size - the size of the data buffer
* @return none
*/
void print_hex(char * data, size_t size) {
	unsigned char *p = (unsigned char *)data;
	for (size_t i = 0; i<size; i++) {
		if ((i % 16 == 0) && i)
			printf("\n");
		printf("%02x", p[i]);
	}
}