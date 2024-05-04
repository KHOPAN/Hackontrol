#include <stdio.h>
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <khopanerror.h>

#define CALL(x,y) result=(x);if(FAILED(result)){KHWin32ConsoleErrorW(result,(y));return 1;}

int main(int argc, char** argv) {
	HRESULT result;
	CALL(CoInitialize(NULL), L"CoInitialize");
	CALL(MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET), L"MFStartup");
	IMFAttributes* attributes;
	CALL(MFCreateAttributes(&attributes, 0), L"MFCreateAttributes");
	CALL(attributes->lpVtbl->SetGUID(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID), L"IMFAttributes::SetGUID");
	IMFActivate** activates;
	UINT32 count;
	CALL(MFEnumDeviceSources(attributes, &activates, &count), L"MFEnumDeviceSources");

	if(count == 0) {
		printf("Error: No devices were found\n");
		return 1;
	}

	IMFActivate* activate = activates[0];
	IMFMediaSource* source;
	CALL(activate->lpVtbl->ActivateObject(activate, &IID_IMFMediaSource, &source), L"IMFActivate::ActivateObject");

	IMFSourceReader* reader;
	CALL(MFCreateSourceReaderFromMediaSource(source, NULL, &reader), L"MFCreateSourceReaderFromMediaSource");
	
	IMFMediaType* mediaType;
	CALL(MFCreateMediaType(&mediaType), L"MFCreateMediaType");
	CALL(mediaType->lpVtbl->SetGUID(mediaType, &MF_MT_MAJOR_TYPE, &MFMediaType_Video), L"IMFMediaType::SetGUID");
	CALL(mediaType->lpVtbl->SetGUID(mediaType, &MF_MT_SUBTYPE, &MFVideoFormat_MJPG), L"IMFMediaType::SetGUID");
	CALL(reader->lpVtbl->SetCurrentMediaType(reader, MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, mediaType), L"IMFSourceReader::SetCurrentMediaType");
	CALL(mediaType->lpVtbl->Release(mediaType), L"IMFMediaType::Release");
	CALL(reader->lpVtbl->GetCurrentMediaType(reader, MF_SOURCE_READER_FIRST_VIDEO_STREAM, &mediaType), L"IMFSourceReader::GetCurrentMediaType");
	UINT64 frameSize;
	CALL(mediaType->lpVtbl->GetUINT64(mediaType, &MF_MT_FRAME_SIZE, &frameSize), L"IMFMediaType::GetUINT64");
	CALL(mediaType->lpVtbl->Release(mediaType), L"IMFMediaType::Release");
	UINT32 width = (UINT32) (frameSize >> 32);
	UINT32 height = (UINT32) frameSize;
	printf("Size: %ux%u\n", width, height);

	IMFSample* sample;
	DWORD streamIndex;
	DWORD flags;
	LONGLONG timestamp;

	while(1) {
		CALL(reader->lpVtbl->ReadSample(reader, MF_SOURCE_READER_ANY_STREAM, 0, &streamIndex, &flags, &timestamp, &sample), L"IMFSourceReader::ReadSample");
		
		if(flags & MF_SOURCE_READERF_STREAMTICK) {
			continue;
		}

		break;
	}

	IMFMediaBuffer* buffer;
	CALL(sample->lpVtbl->ConvertToContiguousBuffer(sample, &buffer), L"IMFMediaBuffer::ConvertToContiguousBuffer");
	BYTE* data;
	DWORD size;
	CALL(buffer->lpVtbl->Lock(buffer, &data, NULL, &size), L"IMFMediaBuffer::Lock");
	HANDLE file = CreateFileW(L"C:\\Users\\puthi\\Downloads\\image.jpg", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if(file == INVALID_HANDLE_VALUE) {
		KHWin32ConsoleErrorW(GetLastError(), L"CreateFileW");
		return 1;
	}
	
	DWORD written;
	
	if(!WriteFile(file, data, size, &written, NULL) || written != size) {
		KHWin32ConsoleErrorW(GetLastError(), L"WriteFile");
		return 1;
	}

	CloseHandle(file);
	CALL(buffer->lpVtbl->Unlock(buffer), L"IMFMediaBuffer::Unlock");
	CALL(buffer->lpVtbl->Release(buffer), L"IMFMediaBuffer::Release");

	CALL(sample->lpVtbl->Release(sample), L"IMFSample::Release");
	CALL(reader->lpVtbl->Release(reader), L"IMFSourceReader::Release");
	return 0;
}
