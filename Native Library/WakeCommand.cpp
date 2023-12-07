#include <WinSock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <jni.h>
#include "StringUtils.h"
#include "ErrorUtils.h"
#include "CommandSource.h"
#include "com_khopan_hackontrol_command_WakeCommand.h"

JNIEXPORT void JNICALL Java_com_khopan_hackontrol_command_WakeCommand_wakeAddress(JNIEnv* environment, jclass wakeCommand, jobject source, jint ipAddress) {
	jclass inetAddressClass = environment->FindClass("java/net/InetAddress");

	if(!inetAddressClass) {
		ErrorUtils_ThrowException(environment, "java/lang/ClassNotFoundException", "Class 'java.net.InetAddress' not found");
		return;
	}

	jmethodID getByAddressMethod = environment->GetStaticMethodID(inetAddressClass, "getByAddress", "([B)Ljava/net/InetAddress;");

	if(!getByAddressMethod) {
		ErrorUtils_ThrowException(environment, "java/lang/NoSuchMethodException", "Static method 'getByAddress([B)Ljava/net/InetAddress;' not found in class 'java.net.InetAddress'");
		return;
	}

	jclass datagramSocketClass = environment->FindClass("java/net/DatagramSocket");

	if(!datagramSocketClass) {
		ErrorUtils_ThrowException(environment, "java/lang/ClassNotFoundException", "Class 'java.net.DatagramSocket' not found");
		return;
	}

	jmethodID datagramSocketConstructor = environment->GetMethodID(datagramSocketClass, "<init>", "()V");

	if(!datagramSocketConstructor) {
		ErrorUtils_ThrowException(environment, "java/lang/NoSuchMethodException", "Constructor 'DatagramSocket()V' not found in class 'java.net.DatagramSocket'");
		return;
	}

	jmethodID sendMethod = environment->GetMethodID(datagramSocketClass, "send", "(Ljava/net/DatagramPacket;)V");

	if(!sendMethod) {
		ErrorUtils_ThrowException(environment, "java/lang/NoSuchMethodException", "Method 'send(Ljava/net/DatagramPacket;)V' not found in class 'java.net.DatagramSocket'");
		return;
	}

	jmethodID closeMethod = environment->GetMethodID(datagramSocketClass, "close", "()V");

	if(!closeMethod) {
		ErrorUtils_ThrowException(environment, "java/lang/NoSuchMethodException", "Method 'close()V' not found in class 'java.net.DatagramSocket'");
		return;
	}

	jclass datagramPacketClass = environment->FindClass("java/net/DatagramPacket");

	if(!datagramPacketClass) {
		ErrorUtils_ThrowException(environment, "java/lang/ClassNotFoundException", "Class 'java.net.DatagramPacket' not found");
		return;
	}

	jmethodID datagramPacketConstructor = environment->GetMethodID(datagramPacketClass, "<init>", "([BILjava/net/InetAddress;I)V");

	if(!datagramPacketConstructor) {
		ErrorUtils_ThrowException(environment, "java/lang/NoSuchMethodException", "Constructor 'DatagramPacket([BILjava/net/InetAddress;I)V' not found in class 'java.net.DatagramPacket'");
		return;
	}

	WSADATA data = {0};
	unsigned long response = WSAStartup(MAKEWORD(2, 2), &data);

	if(response != NO_ERROR) {
		StringUtils_FormatAndSendError(environment, source, response);
		return;
	}

	BYTE address[6];
	ULONG length = 6;
	response = SendARP(ipAddress, NULL, address, &length);

	if(response != NO_ERROR) {
		StringUtils_FormatAndSendError(environment, source, response);
		return;
	}

	jbyte buffer[102];
	memset(buffer, 0xFF, 6);

	for(int i = 6; i < 102; i++) {
		buffer[i] = address[i % 6];
	}

	jbyteArray byteArray = environment->NewByteArray(102);
	environment->SetByteArrayRegion(byteArray, 0, 102, buffer);
	jbyteArray addressArray = environment->NewByteArray(4);
	jbyte addressBytes[4];
	addressBytes[0] = ipAddress & 0xFF;
	addressBytes[1] = (ipAddress >> 8) & 0xFF;
	addressBytes[2] = (ipAddress >> 16) & 0xFF;
	addressBytes[3] = (ipAddress >> 24) & 0xFF;
	environment->SetByteArrayRegion(addressArray, 0, 4, addressBytes);
	jobject inetAddressInstance = environment->CallStaticObjectMethod(inetAddressClass, getByAddressMethod, addressArray);
	jobject datagramPacketInstance = environment->NewObject(datagramPacketClass, datagramPacketConstructor, byteArray, 102, inetAddressInstance, 9);
	jobject datagramSocketInstance = environment->NewObject(datagramSocketClass, datagramSocketConstructor);
	environment->CallVoidMethod(datagramSocketInstance, sendMethod, datagramPacketInstance);
	environment->CallVoidMethod(datagramSocketInstance, closeMethod);
	response = WSACleanup();

	if(response != NO_ERROR) {
		response = WSAGetLastError();
		StringUtils_FormatAndSendError(environment, source, response);
		return;
	}

	jstring string = environment->NewStringUTF("WOL packet sent");
	CommandSource_message(environment, source, string);
}

JNIEXPORT void JNICALL Java_com_khopan_hackontrol_command_WakeCommand_wakeAll
(JNIEnv*, jclass, jobject);
