#include <stdio.h>

#define LIBARCHIVE_STATIC
#include <archive.h>
#include <archive_entry.h>

int main(int argc, char** argv) {
	struct archive* archive = archive_read_new();
	archive_read_support_filter_all(archive);
	archive_read_support_format_all(archive);
	int status = archive_read_open_filename(archive, "D:\\OpenJDK21U-jre_x64_windows_hotspot_21.0.3_9\\java-runtime.7z", 10240);

	if(status != ARCHIVE_OK) {
		printf("archive_read_open_filename() error\n");
		return 1;
	}

	struct archive_entry* entry;

	while(archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
		printf("%s\n", archive_entry_pathname(entry));
		archive_read_data_skip(archive);
	}

	status = archive_read_free(archive);

	if(status != ARCHIVE_OK) {
		printf("archive_read_free() error\n");
		return 1;
	}

	return 0;
}
