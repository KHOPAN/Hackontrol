package com.khopan.hackontrol.remote.session;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import com.khopan.hackontrol.remote.network.Packet;

public class RemoteSession {
	private final Socket socket;
	private final InputStream inputStream;
	private final OutputStream outputStream;
	private final String name;

	public RemoteSession(Socket socket, InputStream inputStream, OutputStream outputStream, String name) {
		this.socket = socket;
		this.inputStream = inputStream;
		this.outputStream = outputStream;
		this.name = name + " (" + this.socket.getInetAddress().getHostAddress() + ')';
	}

	public void start() throws IOException {
		while(true) {
			Packet packet = Packet.readPacket(this.inputStream);
		}
	}

	public void open() {

	}

	@Override
	public String toString() {
		return this.name;
	}
}
