package com.khopan.hackontrol.hrsp;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.function.Consumer;

import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class HRSPClient {
	public HRSPClient(String domainName, int port, Consumer<MessageCreateData> consumer) throws Throwable {
		Socket socket = new Socket(domainName, port);
		OutputStream outputStream = socket.getOutputStream();
		outputStream.write("HRSP 1.0 CONNECT".getBytes(StandardCharsets.UTF_8));
		InputStream inputStream = socket.getInputStream();
		byte[] data = inputStream.readNBytes(11);

		if(!"HRSP 1.0 OK".equals(new String(data, StandardCharsets.UTF_8))) {
			socket.close();
			return;
		}

		consumer.accept(MessageCreateData.fromContent("**Connected**"));
		socket.close();
	}
}
