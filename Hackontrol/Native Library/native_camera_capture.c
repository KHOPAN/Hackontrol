#include <khopanjni.h>
#include <mfapi.h>
#include <Mfidl.h>
#include <mfreadwrite.h>
#include "native_camera.h"
#include "camera_internal.h"

jbyteArray NativeLibrary_capture(JNIEnv* environment, jclass nativeLibraryClass, jobject cameraDeviceInstance, jboolean useMjpg) {
	if(!cameraDeviceInstance) {
		return NULL;
	}
	
	jclass cameraDeviceClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/CameraDevice");

	if(!cameraDeviceClass) {
		KHStandardErrorW(environment, L"Class 'com.khopan.hackontrol.CameraDevice' not found");
		return NULL;
	}

	jfieldID symbolicLinkField = (*environment)->GetFieldID(environment, cameraDeviceClass, "symbolicLink", "Ljava/lang/String;");
	
	if(!symbolicLinkField) {
		KHStandardErrorW(environment, L"Field 'symbolicLink' not found in class 'com.khopan.hackontrol.CameraDevice'");
		return NULL;
	}

	jstring symbolicLinkJava = (jstring) ((*environment)->GetObjectField(environment, cameraDeviceInstance, symbolicLinkField));

	if(!symbolicLinkJava) {
		return NULL;
	}

	LPWSTR symbolicLink = KHJavaToNativeStringW(environment, symbolicLinkJava);

	if(!symbolicLink) {
		return NULL;
	}

	if(!InitializeCamera(environment)) {
		LocalFree(symbolicLink);
		return NULL;
	}

	IMFAttributes* attributes;
	HRESULT result = MFCreateAttributes(&attributes, 2);
	jbyteArray returnResult = NULL;

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFCreateAttributes");
		LocalFree(symbolicLink);
		goto uninitialize;
	}

	result = attributes->lpVtbl->SetGUID(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFAttributes::SetGUID");
		LocalFree(symbolicLink);
		goto releaseAttributes;
	}

	result = attributes->lpVtbl->SetString(attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, symbolicLink);
	LocalFree(symbolicLink);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFAttributes::SetGUID");
		goto releaseAttributes;
	}

	IMFMediaSource* source;
	result = MFCreateDeviceSource(attributes, &source);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFCreateDeviceSource");
		goto releaseAttributes;
	}

	IMFSourceReader* reader;
	result = MFCreateSourceReaderFromMediaSource(source, NULL, &reader);
	HRESULT resultOverlapped = source->lpVtbl->Release(source);

	if(FAILED(resultOverlapped)) {
		KHWin32ErrorW(environment, resultOverlapped, L"IMFMediaSource::Release");
	}

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFCreateSourceReaderFromMediaSource");
		goto releaseAttributes;
	}

	IMFMediaType* mediaType;
	result = MFCreateMediaType(&mediaType);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"MFCreateMediaType");
		goto releaseReader;
	}

	result = mediaType->lpVtbl->SetGUID(mediaType, &MF_MT_MAJOR_TYPE, &MFMediaType_Video);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFMediaType::SetGUID");
		result = mediaType->lpVtbl->Release(mediaType);

		if(FAILED(result)) {
			KHWin32ErrorW(environment, result, L"IMFMediaType::Release");
		}

		goto releaseReader;
	}

	result = mediaType->lpVtbl->SetGUID(mediaType, &MF_MT_SUBTYPE, useMjpg ? &MFVideoFormat_MJPG : &MFVideoFormat_YUY2);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFMediaType::SetGUID");
		result = mediaType->lpVtbl->Release(mediaType);

		if(FAILED(result)) {
			KHWin32ErrorW(environment, result, L"IMFMediaType::Release");
		}

		goto releaseReader;
	}

	result = reader->lpVtbl->SetCurrentMediaType(reader, MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, mediaType);

	if(FAILED(result)) {
		if(result == 0xC00D5212) {
			jclass exceptionClass = (*environment)->FindClass(environment, "java/lang/UnsupportedOperationException");
			
			if(!exceptionClass) {
				KHStandardErrorW(environment, L"Toolchain broken, class 'java.lang.UnsupportedOperationException' not found");
			} else {
				(*environment)->ThrowNew(environment, exceptionClass, "Unsupported format");
			}
		} else {
			KHWin32ErrorW(environment, result, L"IMFSourceReader::SetCurrentMediaType");
		}

		result = mediaType->lpVtbl->Release(mediaType);

		if(FAILED(result)) {
			KHWin32ErrorW(environment, result, L"IMFMediaType::Release");
		}

		goto releaseReader;
	}

	result = mediaType->lpVtbl->Release(mediaType);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFMediaType::Release");
		goto releaseReader;
	}

	UINT64 frameSize;
	reader->lpVtbl->GetCurrentMediaType(reader, MF_SOURCE_READER_FIRST_VIDEO_STREAM, &mediaType);
	mediaType->lpVtbl->GetUINT64(mediaType, &MF_MT_FRAME_SIZE, &frameSize);
	UINT32 width = (UINT32) (frameSize >> 32);
	UINT32 height = (UINT32) frameSize;
	DWORD stream;
	DWORD flags;
	LONGLONG timestamp;
	IMFSample* sample = NULL;

	while(1) {
		result = reader->lpVtbl->ReadSample(reader, MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &stream, &flags, &timestamp, &sample);
	
		if(FAILED(result)) {
			KHWin32ErrorW(environment, result, L"IMFSourceReader::ReadSample");
			goto releaseSample;
		}

		if(flags & MF_SOURCE_READERF_STREAMTICK) {
			continue;
		}

		break;
	}

	IMFMediaBuffer* mediaBuffer;
	result = sample->lpVtbl->ConvertToContiguousBuffer(sample, &mediaBuffer);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFSample::ConvertToContiguousBuffer");
		goto releaseSample;
	}

	BYTE* data;
	DWORD size;
	result = mediaBuffer->lpVtbl->Lock(mediaBuffer, &data, NULL, &size);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFMediaBuffer::Lock");
		goto releaseMediaBuffer;
	}

	jsize dataSize = (jsize) size;
	jsize byteArraySize = dataSize + 2 * sizeof(unsigned long);
	jbyteArray byteArray = (*environment)->NewByteArray(environment, byteArraySize);

	if(!byteArray) {
		goto unlockMediaBuffer;
	}

	unsigned long widthJava = htonl(width);
	unsigned long heightJava = htonl(height);
	(*environment)->SetByteArrayRegion(environment, byteArray, 0, sizeof(unsigned long), (jbyte*) &widthJava);
	(*environment)->SetByteArrayRegion(environment, byteArray, sizeof(unsigned long), sizeof(unsigned long), (jbyte*) &heightJava);
	(*environment)->SetByteArrayRegion(environment, byteArray, 2 * sizeof(unsigned long), dataSize, data);
	returnResult = byteArray;
unlockMediaBuffer:
	result = mediaBuffer->lpVtbl->Unlock(mediaBuffer);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFMediaBuffer::Unlock");
	}
releaseMediaBuffer:
	result = mediaBuffer->lpVtbl->Release(mediaBuffer);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFMediaBuffer::Release");
	}
releaseSample:
	if(sample) {
		result = sample->lpVtbl->Release(sample);

		if(FAILED(result)) {
			KHWin32ErrorW(environment, result, L"IMFSample::Release");
		}
	}
releaseReader:
	result = reader->lpVtbl->Release(reader);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFSourceReader::Release");
	}
releaseAttributes:
	result = attributes->lpVtbl->Release(attributes);

	if(FAILED(result)) {
		KHWin32ErrorW(environment, result, L"IMFAttributes::Release");
	}
uninitialize:
	UninitializeCamera(environment);
	return returnResult;
}
