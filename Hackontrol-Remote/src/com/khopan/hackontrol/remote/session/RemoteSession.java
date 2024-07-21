package com.khopan.hackontrol.remote.session;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import javax.swing.JFrame;
import javax.swing.WindowConstants;

import com.khopan.hackontrol.remote.HackontrolRemote;
import com.khopan.hackontrol.remote.network.Packet;

public class RemoteSession {
	private final Socket socket;
	private final InputStream inputStream;
	private final OutputStream outputStream;
	private final String name;
	private final JFrame frame;

	public RemoteSession(Socket socket, InputStream inputStream, OutputStream outputStream, String name) {
		this.socket = socket;
		this.inputStream = inputStream;
		this.outputStream = outputStream;
		String address = this.socket.getInetAddress().getHostAddress();
		this.name = name + " (" + address + ')';
		this.frame = new JFrame();
		this.frame.setTitle(HackontrolRemote.NAME + " - " + name + ' ' + address);
		this.frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		this.frame.setSize(600, 400);
		this.frame.setLocationRelativeTo(null);
	}

	public void start() throws IOException {
		while(true) {
			Packet packet = Packet.readPacket(this.inputStream);
		}
	}

	public void open() {
		this.frame.setVisible(true);
	}

	@Override
	public String toString() {
		return this.name;
	}
}
