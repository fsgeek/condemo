/*
Copyright 2017 Tony Mason

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <ntstatus.h> // Import OS level status codes
#define WIN32_NO_STATUS (1) // suppress duplicate status declarations
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <winternl.h>

WCHAR *win32_forbidden_names[] = {
	L"CON",
	L"AUX",
	L"PRN",
};

int wmain(int argc, WCHAR *argv[], WCHAR *envp[])
{
	NTSTATUS status;
	HANDLE fHandle = INVALID_HANDLE_VALUE;
	ACCESS_MASK desiredAccess = GENERIC_READ|SYNCHRONIZE;
	OBJECT_ATTRIBUTES oa;
	IO_STATUS_BLOCK iosb;
	UNICODE_STRING fName;
	WCHAR fNameBuffer[128];
	BOOLEAN remove = FALSE;
	ULONG disposition = FILE_CREATE;
	ULONG options = FILE_DIRECTORY_FILE;

	(void)envp;

	/* brute force option check */
	if (argc > 1) {
		/* accept -d --d or /d */
		if ((0 == _wcsicmp(L"-d", argv[1])) ||
			(0 == _wcsicmp(L"/d", argv[1])) ||
			(0 == _wcsicmp(L"--d", argv[1]))) {
			remove = TRUE;
		}
		else {
			printf("Usage: [-d]  to delete the files\n");
			return (-1);
		}
		printf("attempting to %s\n", remove ? "remove" : "add");
	}

	if (remove) {
		disposition = FILE_OPEN;
		options |= FILE_DELETE_ON_CLOSE;
		desiredAccess |= DELETE;
	}

	for (auto index = 0; index < sizeof(win32_forbidden_names) / sizeof(WCHAR *); index++) {
		wcsncpy_s(fNameBuffer, L"\\??\\C:\\test\\", sizeof(fNameBuffer));
		wcsncat_s(fNameBuffer, win32_forbidden_names[index], sizeof(fNameBuffer));
		fName.Length = (USHORT)(sizeof(WCHAR) * wcslen(fNameBuffer));
		fName.MaximumLength = sizeof(fNameBuffer);
		fName.Buffer = fNameBuffer;

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
			disposition, /* create disposition */
			options,  /* create options*/
			NULL, /* no ea buffer */
			0 /* no ea length */
		);

		if (NT_SUCCESS(status)) {
			printf("Successfully %s %wZ\n", remove ? "removed" : "created", fName);
		}
		else switch (status) {
		default:
			printf("Error 0x%x while attempting to create %wZ\n", status, fName);
			break;
		case STATUS_OBJECT_NAME_COLLISION:
			printf("%wZ already exists, skipping\n", fName);
			break;
		case STATUS_OBJECT_NAME_NOT_FOUND:
			printf("%wZ does not exist, skipping\n", fName);
			break;
		}
	}
	return 0;
}