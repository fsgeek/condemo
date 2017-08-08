#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <winternl.h>


int main(int argc, TCHAR **argv, TCHAR **envp)
{
	NTSTATUS status;
	HANDLE fHandle = INVALID_HANDLE_VALUE;
	ACCESS_MASK desiredAccess = GENERIC_READ|SYNCHRONIZE;
	OBJECT_ATTRIBUTES oa;
	IO_STATUS_BLOCK iosb;
	UNICODE_STRING fName;

	(void)argc;
	(void)argv;
	(void)envp;

	RtlInitUnicodeString(&fName, L"\\??\\C:\\test\\CON");
	oa.Length = sizeof(oa);
	oa.RootDirectory = NULL;
	oa.ObjectName = &fName;
	oa.Attributes = OBJ_CASE_INSENSITIVE;
	oa.SecurityDescriptor = NULL; // inherit
	oa.SecurityQualityOfService = NULL; // default

	// see https://msdn.microsoft.com/en-us/library/bb432380(v=vs.85).aspx for documentation on this call
	status = NtCreateFile(
		&fHandle,
		desiredAccess,
		&oa,
		&iosb,
		NULL,
		FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_ARCHIVE, /* initial attributes */
		0, /* no share*/
		FILE_CREATE, /* create disposition */
		FILE_DIRECTORY_FILE,  /* create options*/
		NULL, /* no ea buffer */
		0 /* no ea length */
	);

	if (NT_SUCCESS(status)) {
		printf("Successfully created %wZ\n", fName);
	}
	else {
		printf("Error 0x%x while attempting to create %wZ\n", status, fName);
	}

	return status;
}