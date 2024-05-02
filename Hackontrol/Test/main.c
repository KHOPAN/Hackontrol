#include <stdio.h>
/*#include <direct.h>

#define LIBARCHIVE_STATIC
#include <archive.h>
#include <archive_entry.h>

static int copy_data(struct archive* source, struct archive* destination);*/
#include <khopanstring.h>

int main(int argc, char** argv) {
	LPWSTR result = KHFormatMessageW(L"%S, %S!" /* Weird case-sensitive bug */, "Hello", "world");

	if(!result) {
		printf("Error\n");
		return 1;
	}

	printf("%ws\n", result);
	LocalFree(result);
	/*struct archive* archive = archive_read_new();
	archive_read_support_filter_all(archive);
	archive_read_support_format_all(archive);
	struct archive* external = archive_write_disk_new();
	archive_write_disk_set_options(external, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);
	archive_write_disk_set_standard_lookup(external);

	if(archive_read_open_filename(archive, "D:\\OpenJDK21U-jre_x64_windows_hotspot_21.0.3_9\\java-runtime.7z", 10240)) {
		printf("archive_read_open_filename() error\n");
		return 1;
	}

	if(_chdir("D:\\Temporary")) {
		printf("_chdir() error\n");
		return 1;
	}

	struct archive_entry* entry;

	while(1) {
		int status = archive_read_next_header(archive, &entry);

		if(status == ARCHIVE_EOF) {
			break;
		}

		if(status < ARCHIVE_OK) {
			printf("Error: %s\n", archive_error_string(archive));
		}

		if(status < ARCHIVE_WARN) {
			printf("archive_read_next_header() error\n");
			return 1;
		}

		status = archive_write_header(external, entry);

		if(status < ARCHIVE_OK) {
			printf("Error: %s\n", archive_error_string(external));
		} else if(archive_entry_size(entry) > 0) {
			status = copy_data(archive, external);

			if(status < ARCHIVE_OK) {
				printf("Error: %s\n", archive_error_string(external));
			}

			if(status < ARCHIVE_WARN) {
				printf("copy_data() error\n");
				return 1;
			}
		}

		status = archive_write_finish_entry(external);

		if(status < ARCHIVE_OK) {
			printf("Error: %s\n", archive_error_string(external));
		}

		if(status < ARCHIVE_WARN) {
			printf("archive_write_finish_entry() error\n");
			return 1;
		}
	}

	archive_read_close(archive);
	archive_read_free(archive);
	archive_write_close(external);
	archive_write_free(external);*/
	return 0;
}

/*static int copy_data(struct archive* source, struct archive* destination) {
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
			printf("Error: %s\n", archive_error_string(destination));
			return status;
		}
	}
}*/
