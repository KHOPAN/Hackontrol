#include <khopanerror.h>
#include <khopanstring.h>

#define LIBARCHIVE_STATIC
#include <archive.h>
#include <archive_entry.h>

#include <direct.h>
#include "extractor.h"
#include "initialize.h"
#include "resource.h"

static int copy_data(struct archive*, struct archive*);

BOOL ExtractJRE() {
	HINSTANCE instance = GetProgramInstance();

	if(!instance) {
		KHWin32DialogErrorW(ERROR_INVALID_HANDLE, L"GetProgramInstance");
		return TRUE;
	}

	HRSRC resourceHandle = FindResourceW(instance, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
	
	if(!resourceHandle) {
		KHWin32DialogErrorW(GetLastError(), L"FindResourceW");
		return TRUE;
	}

	DWORD resourceSize = SizeofResource(instance, resourceHandle);
	
	if(!resourceSize) {
		KHWin32DialogErrorW(GetLastError(), L"SizeofResource");
		return TRUE;
	}

	HGLOBAL resource = LoadResource(instance, resourceHandle);

	if(!resource) {
		KHWin32DialogErrorW(GetLastError(), L"LoadResource");
		return TRUE;
	}

	BYTE* data = LockResource(resource);

	if(!data) {
		KHWin32DialogErrorW(GetLastError(), L"LockResource");
		return TRUE;
	}

	BYTE* buffer = LocalAlloc(LMEM_FIXED, resourceSize);

	if(!buffer) {
		KHWin32DialogErrorW(GetLastError(), L"LocalAlloc");
		return TRUE;
	}

	for(DWORD i = 0; i < resourceSize; i++) {
		buffer[i] = (data[i] - 18) % 0xFF;
	}

	struct archive* archive = archive_read_new();
	BOOL error = TRUE;

	if(!archive) {
		KHWin32DialogErrorW(ERROR_OUTOFMEMORY, L"archive_read_new");
		goto freeBuffer;
	}

	archive_read_support_filter_all(archive);
	archive_read_support_format_all(archive);
	struct archive* external = archive_write_disk_new();

	if(!external) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"archive_write_disk_new");
		goto closeArchive;
	}

	archive_write_disk_set_options(external, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);
	archive_write_disk_set_standard_lookup(external);

	if(archive_read_open_memory(archive, buffer, resourceSize)) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"archive_read_open_memory");
		goto closeExternal;
	}

#define TEST_TARGET "D:\\Temporary"

	if(_chdir(TEST_TARGET)) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"_chdir");
		goto closeExternal;
	}

	struct archive_entry* entry;

	while(1) {
		int status = archive_read_next_header(archive, &entry);

		if(status == ARCHIVE_EOF) {
			break;
		}

		if(status < ARCHIVE_OK) {
			MessageBoxA(NULL, archive_error_string(archive), "libarchive Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		}

		if(status < ARCHIVE_WARN) {
			KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"archive_read_next_header");
			goto closeExternal;
		}

		if(archive_write_header(external, entry) >= ARCHIVE_OK && archive_entry_size(entry) > 0) {
			if(copy_data(archive, external) < ARCHIVE_WARN) {
				KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"copy_data");
				goto closeExternal;
			}
		}

		if(archive_write_finish_entry(external) < ARCHIVE_WARN) {
			KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"archive_write_finish_entry");
			goto closeExternal;
		}
	}

	MessageBoxA(NULL, "Extracted the JRE to " TEST_TARGET, "Information", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	error = FALSE;
closeExternal:
	archive_write_close(external);
	archive_write_free(external);
closeArchive:
	archive_read_close(archive);
	archive_read_free(archive);
freeBuffer:
	LocalFree(buffer);
	return error;
}

static int copy_data(struct archive* source, struct archive* destination) {
	const void* buffer;
	size_t size;
	la_int64_t offset;

	while(1) {
		int status = archive_read_data_block(source, &buffer, &size, &offset);

		if(status == ARCHIVE_EOF) {
			return ARCHIVE_OK;
		}

		if(status < ARCHIVE_OK) {
			return status;
		}

		status = (int) archive_write_data_block(destination, buffer, size, offset);

		if(status < ARCHIVE_OK) {
			return status;
		}
	}
}
